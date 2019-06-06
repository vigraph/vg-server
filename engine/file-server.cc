//==========================================================================
// ViGraph dataflow engine server: file-server.cc
//
// HTTP file server for engine server
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "engine.h"
#include "vg-licence.h"
#include "ot-log.h"
#include "ot-web.h"
#include "ot-json.h"

namespace ViGraph { namespace Engine {

namespace
{
  const int default_http_port{33382};
  const string default_index_filename{"index.html"};
  const string server_ident{"ViGraph Engine Server file server"};
}

//==========================================================================
// File URL Handler
// Operations:  GET

class FileURLHandler: public Web::URLHandler
{
  File::Directory dir;
  string index_filename;
  bool handle_get(const Web::HTTPMessage& request, Web::HTTPMessage& response);
  bool handle_request(const Web::HTTPMessage& request,
                      Web::HTTPMessage& response,
                      const SSL::ClientDetails& client);

public:
  FileURLHandler(const File::Directory& _dir, const string& _index_filename):
    URLHandler("/*"), dir(_dir), index_filename(_index_filename)
  {}
};

//--------------------------------------------------------------------------
// Handle a GET request
// Returns whether request was valid
bool FileURLHandler::handle_get(const Web::HTTPMessage& request,
                                Web::HTTPMessage& response)
{
  Log::Streams log;
  log.detail << "File server: GET request for " << request.url << endl;

  auto upath = request.url.get_path();
  if (upath == "/") upath = index_filename;

  auto path = File::Path{dir, upath}.realpath();
  log.detail << "Reading file '" << path << "'\n";
  auto dir_s = dir.str();
  auto path_s = path.str();
  if (path_s.substr(0, dir_s.size()) != dir_s)
  {
    Log::Error log;
    log << "File server received request for file outside directory "
        << path << endl;
    response.code = 404;
    response.reason = "Not found";
  }
  else if (path.is_dir())
  {
    Log::Error log;
    log << "File server trying to read directory " << path << endl;
    response.code = 403;
    response.reason = "Forbidden";
  }
  else if (!path.read_all(response.body))
  {
    Log::Error log;
    log << "File server can't read file " << path
        << ": " << response.body << endl;
    response.code = 404;
    response.reason = "Not found";
  }

  // Set Content-Type according to file suffix (for IE)
  auto ext = path.extension();
  if (ext == "js")
    response.headers.put("Content-Type", "application/js");
  else if (ext == "html")
    response.headers.put("Content-Type", "text/html");
  else if (ext == "css")
    response.headers.put("Content-Type", "text/css");

  return true;
}

//--------------------------------------------------------------------------
// Handle the request
bool FileURLHandler::handle_request(const Web::HTTPMessage& request,
                                    Web::HTTPMessage& response,
                                    const SSL::ClientDetails& /* client */)
{
  if (request.method == "GET")
  {
    if (!handle_get(request, response))
    {
      response.code = 400;
      response.reason = "Bad request";
    }
  }
  else
  {
    response.code = 405;
    response.reason = "Method not allowed";
  }
  return true;
}

//==========================================================================
// File server
//--------------------------------------------------------------------------
// Constructor
FileServer::FileServer(const XML::Element& config,
                       const File::Directory& base_dir)
{
  Log::Streams log;
  XML::ConstXPathProcessor xpath(config);

  // Start HTTP server
  int hport = xpath.get_value_int("http/@port", default_http_port);
  auto dir_s = xpath.get_value("file/@path");
  if (dir_s.empty())
  {
    log.error << "File server: no file path specified" << endl;
    return;
  }

  const auto dir = File::Directory{base_dir}.resolve(dir_s).realpath();
  log.summary << "File server: Starting HTTP server at port " << hport << endl;
  log.summary << "File server: Serving files from " << dir << endl;
  http_server.reset(new Web::SimpleHTTPServer(hport, server_ident));

  const auto ifn = xpath.get_value("index/@file", default_index_filename);
  http_server->add(new FileURLHandler(dir, ifn));

  // Allow cross-origin fetch from anywhere
  http_server->set_cors_origin();

  // Background thread to run it
  http_server_thread.reset(new Net::TCPServerThread(*http_server));
  http_server_thread->detach();
}

//--------------------------------------------------------------------------
// Destructor
FileServer::~FileServer()
{
  if (http_server.get()) http_server->shutdown();
  http_server_thread.reset();
  http_server.reset();
}

}} // namespaces
