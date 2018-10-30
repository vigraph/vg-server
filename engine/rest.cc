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
// URLHandler

//--------------------------------------------------------------------------
// Handle a POST request
// Returns whether request was valid
bool RESTURLHandler::handle_post(const Web::HTTPMessage& request)
{
  Log::Streams log;
  log.detail << "REST: POST request\n";

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
bool RESTURLHandler::handle_delete(const Web::HTTPMessage& /* request */)
{
  Log::Streams log;
  log.detail << "REST: DELETE request\n";

  // !!!

  return true;
}

//--------------------------------------------------------------------------
// Handle the request
bool RESTURLHandler::handle_request(const Web::HTTPMessage& request,
                                    Web::HTTPMessage& response,
                                    const SSL::ClientDetails& /* client */)
{
  const auto& fullpath = request.url.get_path();
  if (fullpath.size() < prefix.size()) return false;  // Shouldn't happen
  // const auto& path = string(fullpath, prefix.size());
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

  const auto& prefix = xpath["url/@prefix"];
  http_server->add(new RESTURLHandler(prefix));

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
