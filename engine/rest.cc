//==========================================================================
// ViGraph dataflow engine server: rest.cc
//
// HTTP REST interface for engine server
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "engine.h"
#include "vg-licence.h"
#include "vg-json.h"
#include "ot-log.h"
#include "ot-web.h"

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
// GET parameters:
//    transient = <anything>  Get dynamic values on inputs
//    recursive = <anything>  Recurse to subgraphs / clones
class GraphURLHandler: public Web::URLHandler
{
  Dataflow::Engine& engine;
  bool handle_get(const string& path, const Web::HTTPMessage& request,
                  Web::HTTPMessage& response);
  bool handle_put(const string& path, const Web::HTTPMessage& request,
                   Web::HTTPMessage& response);
  bool handle_post(const string& path, const Web::HTTPMessage& request,
                   Web::HTTPMessage& response);
  bool handle_delete(const string& path, const Web::HTTPMessage& request,
                     Web::HTTPMessage& response);
  bool handle_request(const Web::HTTPMessage& request,
                      Web::HTTPMessage& response,
                      const SSL::ClientDetails& client);

public:
  GraphURLHandler(Dataflow::Engine& _engine):
    URLHandler("/graph*"), engine(_engine)
  {}
};

//--------------------------------------------------------------------------
// Handle a GET request
// Returns whether request was valid
bool GraphURLHandler::handle_get(const string& path,
                                 const Web::HTTPMessage& request,
                                 Web::HTTPMessage& response)
{
  Log::Streams log;
  log.detail << "REST Graph: GET " << path << endl;

  try
  {
    auto json = JSON::Value{JSON::Value::Type::OBJECT};
    JSON::get(engine, json, path,
              !request.url.get_query_parameter("recursive").empty(),
              !request.url.get_query_parameter("transient").empty());
    if (!json)
    {
      response.code = 404;
      response.reason = "Not found";
    }
    else response.body = json.str(true);
  }
  catch (runtime_error& e)
  {
    Log::Error log;
    log << "REST Graph GET failed: " << e.what() << endl;
    response.code = 404;
    response.reason = "Not found";
  }
  return true;
}

//--------------------------------------------------------------------------
// Handle a PUT request
// Returns whether request was valid
bool GraphURLHandler::handle_put(const string& path,
                                 const Web::HTTPMessage& request,
                                 Web::HTTPMessage& response)
{
  Log::Streams log;
  log.detail << "REST Graph: PUT " << path << endl;

  // Parse JSON
  istringstream iss(request.body);
  ObTools::JSON::Parser parser(iss);
  JSON::Value value;
  try
  {
    value = parser.read_value();
  }
  catch (ObTools::JSON::Exception& e)
  {
    log.error << "REST: JSON parsing failed: " << e.error << endl;
    return false;
  }

  try
  {
    JSON::set(engine, value, path);
  }
  catch (runtime_error& e)
  {
    Log::Error log;
    log << "REST Graph POST failed: " << e.what() << endl;
    response.code = 404;
    response.reason = "Not found";
  }

  return true;
}

//--------------------------------------------------------------------------
// Handle a POST request
// Returns whether request was valid
bool GraphURLHandler::handle_post(const string& path,
                                  const Web::HTTPMessage& request,
                                  Web::HTTPMessage& response)
{
  Log::Streams log;
  log.detail << "REST Graph: POST " << path << endl;

  // Parse JSON
  istringstream iss(request.body);
  ObTools::JSON::Parser parser(iss);
  JSON::Value value;
  try
  {
    value = parser.read_value();
  }
  catch (ObTools::JSON::Exception& e)
  {
    log.error << "REST: JSON parsing failed: " << e.error << endl;
    return false;
  }

  try
  {
    JSON::set(engine, value, path);
  }
  catch (runtime_error& e)
  {
    Log::Error log;
    log << "REST Graph POST failed: " << e.what() << endl;
    response.code = 404;
    response.reason = "Not found";
  }

  return true;
}

//--------------------------------------------------------------------------
// Handle a DELETE request
// Returns whether request was valid
bool GraphURLHandler::handle_delete(const string& path,
                                    const Web::HTTPMessage& /*request*/,
                                    Web::HTTPMessage& response)
{
  Log::Streams log;
  log.detail << "REST Graph: DELETE " << path << endl;

  try
  {
    JSON::del(engine, path);
  }
  catch (runtime_error& e)
  {
    Log::Error log;
    log << "REST Graph DELETE failed: " << e.what() << endl;
    response.code = 404;
    response.reason = "Not found";
  }

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
  if (!path.empty() && path[0] == '/') path.erase(path.begin());

  if (request.method == "GET")
  {
    if (!handle_get(path, request, response))
    {
      response.code = 400;
      response.reason = "Bad request";
    }
  }
  else if (request.method == "PUT")
  {
    if (!handle_put(path, request, response))
    {
      response.code = 400;
      response.reason = "Bad request";
    }
  }
  else if (request.method == "POST")
  {
    if (!handle_post(path, request, response))
    {
      response.code = 400;
      response.reason = "Bad request";
    }
  }
  else if (request.method == "DELETE")
  {
    if (!handle_delete(path, request, response))
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
// /meta/<section>            Metadata for specific section
// /meta/<section>/<module>   Metadata for specific module

class MetaURLHandler: public Web::URLHandler
{
  Dataflow::Engine& engine;
  bool handle_get(const string& path, Web::HTTPMessage& response);
  bool handle_request(const Web::HTTPMessage& request,
                      Web::HTTPMessage& response,
                      const SSL::ClientDetails& client);

public:
  MetaURLHandler(Dataflow::Engine& _engine):
    URLHandler("/meta*"), engine(_engine)
  {}
};

//--------------------------------------------------------------------------
// Handle a GET request
// Returns whether request was valid
bool MetaURLHandler::handle_get(const string& path,
                                Web::HTTPMessage& response)
{
  Log::Streams log;
  Dataflow::Registry& registry = engine.element_registry;

  try
  {
    if (path.empty())
    {
      log.detail << "REST Meta: GET request\n";
      JSON::Value json(JSON::Value::OBJECT);
      for (const auto& sit: registry.sections)
      {
        auto& section_json = json.put(sit.first, JSON::Value::OBJECT);
        for (const auto& mit: sit.second.modules)
        {
          const auto& mod = *mit.second->get_module();
          const auto mod_json = JSON::get_module_metadata(mod);
          section_json.put(mod.get_id(), mod_json);
        }
      }
      response.body = json.str(true);
    }
    else
    {
      log.detail << "REST Meta: GET request for " << path << endl;
      vector<string> bits = Text::split(path, '/');
      if (bits.size() < 1 || bits.size() > 2)
      {
        log.error << "Specific /meta requests require <section>/<id>\n";
        response.code = 404;
        response.reason = "Not found";
      }

      const auto sit = registry.sections.find(bits[0]);
      if (sit == registry.sections.end())
      {
        log.error << "No such section '" << bits[0]
                  << "' requested in REST /meta\n";
        response.code = 404;
        response.reason = "Not found";
      }
      else
      {
        if (bits.size() < 2)
        {
          JSON::Value json(JSON::Value::OBJECT);
          for (const auto& mit: sit->second.modules)
          {
            const auto& mod = *mit.second->get_module();
            const auto mod_json = JSON::get_module_metadata(mod);
            json.put(mod.get_id(), mod_json);
          }
          response.body = json.str(true);
        }
        else
        {
          const auto mit = sit->second.modules.find(bits[1]);
          if (mit == sit->second.modules.end())
          {
            log.error << "No such module '" << bits[1]
                      << "' in section '" << bits[0]
                      << "' requested in REST /meta\n";
            response.code = 404;
            response.reason = "Not found";
          }
          else
          {
            const auto& mod = *mit->second->get_module();
            const auto json = JSON::get_module_metadata(mod);
            response.body = json.str(true);
          }
        }
      }
    }
  }
  catch (ObTools::JSON::Exception e)
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
// /layout URL Handler
// Operations:  GET, POST

// URL format:
// /layout                     Entire top-level layout (GET/POST)

class LayoutURLHandler: public Web::URLHandler
{
  Layout& layout;
  bool handle_get(Web::HTTPMessage& response);
  bool handle_post(const Web::HTTPMessage& request,
                   Web::HTTPMessage& response);
  bool handle_request(const Web::HTTPMessage& request,
                      Web::HTTPMessage& response,
                      const SSL::ClientDetails& client);

public:
  LayoutURLHandler(Layout& _layout):
    URLHandler("/layout*"), layout(_layout)
  {}
};

//--------------------------------------------------------------------------
// Handle a GET request
// Returns whether request was valid
bool LayoutURLHandler::handle_get(Web::HTTPMessage& response)
{
  Log::Streams log;
  log.detail << "REST Layout: GET" << endl;
  response.body = layout.get_layout();
  return true;
}

//--------------------------------------------------------------------------
// Handle a POST request
// Returns whether request was valid
bool LayoutURLHandler::handle_post(const Web::HTTPMessage& request,
                                   Web::HTTPMessage&)
{
  Log::Streams log;
  log.detail << "REST Layout: POST" << endl;
  layout.set_layout(request.body);
  return true;
}

//--------------------------------------------------------------------------
// Handle the request
bool LayoutURLHandler::handle_request(const Web::HTTPMessage& request,
                                      Web::HTTPMessage& response,
                                      const SSL::ClientDetails& /* client */)
{
  if (request.method == "GET")
  {
    if (!handle_get(response))
    {
      response.code = 400;
      response.reason = "Bad request";
    }
  }
  else if (request.method == "POST")
  {
    if (!handle_post(request, response))
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
// /combined URL Handler
// Operations:  GET, POST

// URL format:
// /combined                     Entire top-level combined (GET/POST)

class CombinedURLHandler: public Web::URLHandler
{
  Dataflow::Engine& engine;
  Layout &layout;
  bool handle_get(Web::HTTPMessage& response);
  bool handle_post(const Web::HTTPMessage& request,
                   Web::HTTPMessage& response);
  bool handle_request(const Web::HTTPMessage& request,
                      Web::HTTPMessage& response,
                      const SSL::ClientDetails& client);

public:
  CombinedURLHandler(Dataflow::Engine& _engine, Layout& _layout):
    URLHandler("/combined*"), engine(_engine), layout(_layout)
  {}
};

//--------------------------------------------------------------------------
// Handle a GET request
// Returns whether request was valid
bool CombinedURLHandler::handle_get(Web::HTTPMessage& response)
{
  // Saving enabled?
  if (!engine.is_saving_enabled())
  {
    Log::Error log;
    log << "Combined saving disabled\n";
    response.code = 403;
    response.reason = "Forbidden";
    return true;
  }

  Log::Streams log;
  log.detail << "REST Combined: GET" << endl;

  auto json = JSON::Value{JSON::Value::Type::OBJECT};
  auto& graph = json.put("graph", JSON::Value{JSON::Value::Type::OBJECT});
  JSON::get(engine, graph, Dataflow::Path(""), true, false); // recursive

  string layout_text = layout.get_layout();

  // Parse JSON
  istringstream iss(layout_text);
  ObTools::JSON::Parser parser(iss);
  JSON::Value layout;
  try
  {
    layout = parser.read_value();
  }
  catch (ObTools::JSON::Exception& e)
  {
    log.error << "REST: JSON parsing failed: " << e.error << endl;
    return false;
  }

  json.set("layout", layout);
  response.body = json.str();
  response.headers.put("Content-Type", "text/json");
  response.headers.put("Content-Disposition",
                       "attachment; filename=vigraph.json");

  return true;
}

//--------------------------------------------------------------------------
// Handle a POST request
// Returns whether request was valid
bool CombinedURLHandler::handle_post(const Web::HTTPMessage& request,
                                   Web::HTTPMessage&)
{
  Log::Streams log;
  log.detail << "REST Combined: POST" << endl;

  // Parse JSON
  istringstream iss(request.body);
  ObTools::JSON::Parser parser(iss);
  JSON::Value combined;
  try
  {
    combined = parser.read_value();
  }
  catch (ObTools::JSON::Exception& e)
  {
    log.error << "REST: JSON parsing failed: " << e.error << endl;
    return false;
  }

  const auto& graph = combined["graph"];
  if (graph.type == JSON::Value::Type::OBJECT)
  {
    try
    {
      JSON::set(engine, graph, Dataflow::Path(""));
    }
    catch (runtime_error& e)
    {
      Log::Error log;
      log << "REST combined graph setting failed: " << e.what() << endl;
    }
  }

  const auto& lo = combined["layout"];
  if (lo.type == JSON::Value::Type::OBJECT)
    layout.set_layout(lo.str());

  return true;
}

//--------------------------------------------------------------------------
// Handle the request
bool CombinedURLHandler::handle_request(const Web::HTTPMessage& request,
                                      Web::HTTPMessage& response,
                                      const SSL::ClientDetails& /* client */)
{
  if (request.method == "GET")
  {
    if (!handle_get(response))
    {
      response.code = 400;
      response.reason = "Bad request";
    }
  }
  else if (request.method == "POST")
  {
    if (!handle_post(request, response))
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
// /version URL Handler
// Operations:  GET

// URL format:
// /version                   Version information

class VersionURLHandler: public Web::URLHandler
{
  Dataflow::Engine& engine;
  bool handle_get(Web::HTTPMessage& response);
  bool handle_request(const Web::HTTPMessage& request,
                      Web::HTTPMessage& response,
                      const SSL::ClientDetails& client);

public:
  VersionURLHandler(Dataflow::Engine& _engine):
    URLHandler("/version"), engine(_engine)
  {}
};

//--------------------------------------------------------------------------
// Handle a GET request
// Returns whether request was valid
bool VersionURLHandler::handle_get(Web::HTTPMessage& response)
{
  Log::Streams log;
  log.detail << "REST Version: GET request\n";
  JSON::Value json(JSON::Value::OBJECT);
  json.put("major", 2);
  json.put("minor", 0);
  json.put("name", "TBC");
  json.put("saving", engine.is_saving_enabled());
  response.body = json.str(true);
  return true;
}

//--------------------------------------------------------------------------
// Handle the request
bool VersionURLHandler::handle_request(const Web::HTTPMessage& request,
                                       Web::HTTPMessage& response,
                                       const SSL::ClientDetails& /* client */)
{
  if (request.method == "GET")
  {
    if (!handle_get(response))
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
RESTInterface::RESTInterface(const XML::Element& config,
                             Dataflow::Engine& engine,
                             const File::Directory&)
{
  Log::Streams log;
  XML::ConstXPathProcessor xpath(config);

  // Start HTTP server
  int hport = xpath.get_value_int("http/@port", default_http_port);
  log.summary << "REST: Starting HTTP server at port " << hport << endl;
  http_server.reset(new Web::SimpleHTTPServer(hport, server_ident));

  http_server->add(new GraphURLHandler(engine));
  http_server->add(new MetaURLHandler(engine));
  http_server->add(new LayoutURLHandler(layout));
  http_server->add(new CombinedURLHandler(engine, layout));
  http_server->add(new VersionURLHandler(engine));

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
