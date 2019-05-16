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
  const string default_layout_file{"layout.json"};
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
  Dataflow::Engine& engine;
  JSON::Value get_json_for_element(const Dataflow::Element& e);
  bool handle_get(const string& path, Web::HTTPMessage& response);
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
// Get JSON for a graph element
JSON::Value GraphURLHandler::get_json_for_element(const Dataflow::Element& e)
{
  JSON::Value json(JSON::Value::OBJECT);
  json.set("id", e.id);
  json.set("type", e.module->id);
  return json;
}

//--------------------------------------------------------------------------
// Handle a GET request
// Returns whether request was valid
bool GraphURLHandler::handle_get(const string& path,
                                 Web::HTTPMessage& response)
{
  Log::Streams log;
  log.detail << "REST Graph: GET " << path << endl;

  try
  {
    JSON::Value json = engine.get_json(path);
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

  try
  {
    engine.add_json(path, value);
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

  try
  {
    engine.set_json(path, value);
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
    engine.delete_item(path);
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
    if (!handle_get(path, response))
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
// /meta/<type>               Metadata for specific type

class MetaURLHandler: public Web::URLHandler
{
  Dataflow::Engine& engine;
  bool handle_get(const string& path, Web::HTTPMessage& response);
  bool handle_request(const Web::HTTPMessage& request,
                      Web::HTTPMessage& response,
                      const SSL::ClientDetails& client);
  JSON::Value get_metadata_for_module(const Dataflow::Module& module);

public:
  MetaURLHandler(Dataflow::Engine& _engine):
    URLHandler("/meta*"), engine(_engine)
  {}
};

//--------------------------------------------------------------------------
// Get metadata JSON for a particular module
JSON::Value MetaURLHandler::get_metadata_for_module(
                                           const Dataflow::Module& module)
{
  JSON::Value json(JSON::Value::OBJECT);
  json.set("id", module.id);
  json.set("name", module.name);
  json.set("description", module.description);
  json.set("section", module.section);

  // Properties
  if (!module.properties.empty())
  {
    JSON::Value propsj(JSON::Value::ARRAY);
    JSON::Value ipropsj(JSON::Value::ARRAY);
    for(const auto pit: module.properties)
    {
      const auto& prop = pit.second;
      JSON::Value& pj =
        (prop.settable?ipropsj:propsj).add(JSON::Value(JSON::Value::OBJECT));
      pj.set("id", pit.first);
      pj.set("description", prop.description);
      if (prop.type == Dataflow::Value::Type::other)
        pj.set("type", prop.other_type);
      else
        pj.set("type", Dataflow::Value::type_str(prop.type));
      if (prop.alias)
        pj.set("alias", JSON::Value(JSON::Value::TRUE_));
    }

    if (!propsj.a.empty()) json.set("props", propsj);
    if (!ipropsj.a.empty()) json.set("iprops", ipropsj);
  }

  // Controlled properties
  if (!module.controlled_properties.empty())
  {
    JSON::Value& propsj = json.put("oprops",
                                   JSON::Value(JSON::Value::ARRAY));
    for(const auto pit: module.controlled_properties)
    {
      JSON::Value& pj = propsj.add(JSON::Value(JSON::Value::OBJECT));
      pj.set("id", pit.first);
      const auto& prop = pit.second;
      pj.set("description", prop.description);
      pj.set("type", Dataflow::Value::type_str(prop.type));
    }
  }

  // Inputs
  if (!module.inputs.empty())
  {
    JSON::Value& inputsj = json.put("inputs", JSON::Value(JSON::Value::ARRAY));
    for(const auto input: module.inputs)
    {
      JSON::Value& ij = inputsj.add(JSON::Value(JSON::Value::OBJECT));
      ij.set("type", input.type);
      if (input.multiple)
        ij.set("multiple", JSON::Value(JSON::Value::TRUE_));
    }
  }

  // Outputs
  if (!module.outputs.empty())
  {
    JSON::Value& outsj = json.put("outputs", JSON::Value(JSON::Value::ARRAY));
    for(const auto output: module.outputs)
    {
      JSON::Value& oj = outsj.add(JSON::Value(JSON::Value::OBJECT));
      oj.set("type", output.type);
      if (output.multiple)
        oj.set("multiple", JSON::Value(JSON::Value::TRUE_));
    }
  }

  return json;
}

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
      JSON::Value json(JSON::Value::ARRAY);
      for(const auto& sit: registry.sections)
        for(const auto& mit: sit.second.modules)
          json.add(get_metadata_for_module(*mit.second.module));
      response.body = json.str(true);
    }
    else
    {
      log.detail << "REST Meta: GET request for " << path << endl;
      vector<string> bits = Text::split(path, '/');
      if (bits.size() < 2)
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
          JSON::Value json = get_metadata_for_module(*mit->second.module);
          response.body = json.str(true);
        }
      }
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
// /layout URL Handler
// Operations:  GET, POST

// URL format:
// /layout                     Entire top-level layout (GET/POST)

class LayoutURLHandler: public Web::URLHandler
{
  File::Path layout_path;
  bool handle_get(Web::HTTPMessage& response);
  bool handle_post(const Web::HTTPMessage& request,
                   Web::HTTPMessage& response);
  bool handle_request(const Web::HTTPMessage& request,
                      Web::HTTPMessage& response,
                      const SSL::ClientDetails& client);

public:
  LayoutURLHandler(const File::Path& _layout_path):
    URLHandler("/layout*"), layout_path(_layout_path)
  {}
};

//--------------------------------------------------------------------------
// Handle a GET request
// Returns whether request was valid
bool LayoutURLHandler::handle_get(Web::HTTPMessage& response)
{
  Log::Streams log;
  log.detail << "REST Layout: GET from " << layout_path << endl;

  if (!layout_path.read_all(response.body))
  {
    Log::Error log;
    log << "REST Layout can't read file " << layout_path
        << ": " << response.body << endl;
    response.code = 404;
    response.reason = "Not found";
  }
  return true;
}

//--------------------------------------------------------------------------
// Handle a POST request
// Returns whether request was valid
bool LayoutURLHandler::handle_post(const Web::HTTPMessage& request,
                                   Web::HTTPMessage& response)
{
  Log::Streams log;
  log.detail << "REST Layout: POST to " << layout_path << endl;

  const auto& error = layout_path.write_all(request.body);
  if (!error.empty())
  {
    Log::Error log;
    log << "REST Layout can't write file " << layout_path
        << ": " << error << endl;
    response.code = 403;
    response.reason = "Forbidden";
  }
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
// REST interface
//--------------------------------------------------------------------------
// Constructor
RESTInterface::RESTInterface(const XML::Element& config,
                             Dataflow::Engine& engine,
                             const File::Directory& base_dir)
{
  Log::Streams log;
  XML::ConstXPathProcessor xpath(config);

  // Config
  const auto& layout_file = config.get_child("layout")
                                  .get_attr("file", default_layout_file);

  // Start HTTP server
  int hport = xpath.get_value_int("http/@port", default_http_port);
  log.summary << "REST: Starting HTTP server at port " << hport << endl;
  http_server.reset(new Web::SimpleHTTPServer(hport, server_ident));

  http_server->add(new GraphURLHandler(engine));
  http_server->add(new MetaURLHandler(engine));
  http_server->add(new LayoutURLHandler(base_dir.resolve(layout_file)));

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
