//==========================================================================
// ViGraph dataflow engine server: rest.cc
//
// HTTP REST interface for engine server
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "engine.h"
#include "vg-licence.h"
#include "ot-log.h"
#include "ot-web.h"
#include "ot-json.h"

namespace ViGraph { namespace Engine {

namespace
{
  const int default_http_port{33380};
  const string server_ident{"ViGraph Engine Server REST interface"};
}

//==========================================================================
// /graph URL Handler
// Operations:  GET, PUT, POST, DELETE

// URL format:
// /graph                     Entire top-level graph (GET only!)
// /graph/<id>                Top-level element by <id>
// /graph/<id>/<sid>...       Next-level element by ID hierarchy
// /graph/<id>.<prop>         Property or output of top level element
// /graph/<id>/<sid>.<prop>   Property or output of nested element
// /graph/<id>/<prop>.<sprop> Nested property of element

class GraphURLHandler: public Web::URLHandler
{
  bool handle_delete(const Web::HTTPMessage& request);
  bool handle_post(const Web::HTTPMessage& request);
  bool handle_request(const Web::HTTPMessage& request,
                      Web::HTTPMessage& response,
                      const SSL::ClientDetails& client);

public:
  GraphURLHandler(): URLHandler("/graph*")
  {}
};

//--------------------------------------------------------------------------
// Handle a POST request
// Returns whether request was valid
bool GraphURLHandler::handle_post(const Web::HTTPMessage& request)
{
  Log::Streams log;
  log.detail << "REST Graph: POST request\n";

  // Parse JSON
  istringstream iss(request.body);
  JSON::Parser parser(iss);
  JSON::Value value;
  try
  {
    value = parser.read_value();
  }
  catch (JSON::Exception& e)
  {
    log.error << "REST: JSON parsing failed: " << e.error << endl;
    return false;
  }

  // !!!

  return true;
}

//--------------------------------------------------------------------------
// Handle a DELETE request
// Returns whether request was valid
bool GraphURLHandler::handle_delete(const Web::HTTPMessage& /* request */)
{
  Log::Streams log;
  log.detail << "REST Graph: DELETE request\n";

  // !!!

  return true;
}

//--------------------------------------------------------------------------
// Handle the request
bool GraphURLHandler::handle_request(const Web::HTTPMessage& request,
                                    Web::HTTPMessage& response,
                                    const SSL::ClientDetails& /* client */)
{
  const auto& fullpath = request.url.get_path();
  if (fullpath.size() < 6) return false;  // minimum /graph - shouldn't happen
  auto path = string(fullpath, 6);
  if (!path.empty() && path[0] == '/') path.erase(0);
  // const auto& bits = Text::split(path, '/');

  if (request.method == "POST")
  {
    if (!handle_post(request))
    {
      response.code = 400;
      response.reason = "Bad request";
    }
  }
  else if (request.method == "DELETE")
  {
    if (!handle_delete(request))
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
// /meta URL Handler
// Operations:  GET

// URL format:
// /meta                      Total metadata structure
// /meta/<type>               Metadata for specific type

class MetaURLHandler: public Web::URLHandler
{
  bool handle_get(const string& path, Web::HTTPMessage& response);
  bool handle_request(const Web::HTTPMessage& request,
                      Web::HTTPMessage& response,
                      const SSL::ClientDetails& client);

public:
  MetaURLHandler(): URLHandler("/meta*")
  {}
};

//--------------------------------------------------------------------------
// Handle a GET request
// Returns whether request was valid
bool MetaURLHandler::handle_get(const string& path,
                                Web::HTTPMessage& response)
{
  Log::Streams log;

  try
  {
    if (path.empty())
    {
      log.detail << "REST Meta: GET request\n";
    }
    else
    {
      JSON::Value json(JSON::Value::OBJECT);
      log.detail << "REST Meta: GET request for " << path << endl;

      // const auto& bits = Text::split(path, '/');

      // !!!
      json.set("path", path);
      response.body = json.str();
    }
  }
  catch (JSON::Exception e)
  {
    log.error << "JSON output error: " << e.error << endl;
  }

  return true;
}

//--------------------------------------------------------------------------
// Handle the request
bool MetaURLHandler::handle_request(const Web::HTTPMessage& request,
                                    Web::HTTPMessage& response,
                                    const SSL::ClientDetails& /* client */)
{
  const auto& fullpath = request.url.get_path();
  if (fullpath.size() < 5) return false;  // minimum /meta - shouldn't happen
  auto path = string(fullpath, 5);
  if (!path.empty() && path[0] == '/') path.erase(path.begin());

  if (request.method == "GET")
  {
    if (!handle_get(path, response))
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
// REST interface
//--------------------------------------------------------------------------
// Constructor
RESTInterface::RESTInterface(const XML::Element& config)
{
  Log::Streams log;
  XML::ConstXPathProcessor xpath(config);

   // Start HTTP server
  int hport = xpath.get_value_int("http/@port", default_http_port);
  log.summary << "REST: Starting HTTP server at port " << hport << endl;
  http_server.reset(new Web::SimpleHTTPServer(hport, server_ident));

  http_server->add(new GraphURLHandler());
  http_server->add(new MetaURLHandler());

  // Allow cross-origin fetch from anywhere
  http_server->set_cors_origin();

  // Background thread to run it
  http_server_thread.reset(new Net::TCPServerThread(*http_server));
  http_server_thread->detach();
}

//--------------------------------------------------------------------------
// Destructor
RESTInterface::~RESTInterface()
{
  if (http_server.get()) http_server->shutdown();
  http_server_thread.reset();
  http_server.reset();
}

}} // namespaces
