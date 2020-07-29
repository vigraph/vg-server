//==========================================================================
// Singleton server object for engine service
//
// Copyright (c) 2017-2020 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-service.h"
#include "vg-compiler.h"
#include "vg-json.h"
#include <SDL.h>

namespace ViGraph { namespace Service {

const auto default_section = "core";

//--------------------------------------------------------------------------
// Constructor
Server::Server()
{
}

//--------------------------------------------------------------------------
// Read settings from configuration
void Server::read_config(const XML::Configuration& _config,
                         const string& config_filename)
{
  config_xml = _config.get_root();
  config_file = File::Path(config_filename);
}

//--------------------------------------------------------------------------
// Prerun function - run with original user (usually root) privileges
int Server::run_priv()
{
  return 0;
}

//--------------------------------------------------------------------------
// Main loop function
int Server::pre_run()
{
  Log::Streams log;

  // Configure permanent and transient state
  if (!configure())
  {
    log.error << "Cannot configure server" << endl;
    return 2;
  }

  return 0;
}

//--------------------------------------------------------------------------
// Main loop tick
int Server::tick()
{
  Time::Duration now = Time::Duration::clock();
  if (graph_file_time)
  {
    const auto last_mod = graph_file.last_modified();
    if (last_mod != graph_file_time &&
        last_mod < Time::Stamp::now().time() - 2) // give time for file save
      load_graph(graph_file);
  }
  engine.tick(now);
  SDL_PumpEvents();

  // Handle queued functions
  MT::Lock lock{functions_mutex};
  while (!functions.empty())
  {
    auto ff = move(functions.front());
    functions.pop();
    try
    {
      ff.func();
      ff.promise.set_value(true);
    }
    catch (...)
    {
      ff.promise.set_exception(current_exception());
    }
  }

  return 0;
}

//--------------------------------------------------------------------------
// Run a function on the main thread
future<bool> Server::run_function(function<void()> func)
{
  MT::Lock lock{functions_mutex};
  functions.emplace(func);
  return functions.back().get_future();
}

//--------------------------------------------------------------------------
// Global configuration - called only at startup
// Returns whether successful
bool Server::configure()
{
  Log::Streams log;
  log.summary << "Configuring server permanent state" << endl;

  // Initialise SDL
  SDL_version linked;
  SDL_GetVersion(&linked);
  log.detail << "Loaded SDL library version: "
             << static_cast<int>(linked.major) << "."
             << static_cast<int>(linked.minor) << "."
             << static_cast<int>(linked.patch)  << endl;
  SDL_version compiled;
  SDL_VERSION(&compiled);
  if (compiled.major != linked.major ||
      compiled.minor != linked.minor ||
      compiled.patch != linked.patch)
  {
    log.summary << "SDL compiled version: "
                << static_cast<int>(compiled.major) << "."
                << static_cast<int>(compiled.minor) << "."
                << static_cast<int>(compiled.patch)
                << ", linked version: "
                << static_cast<int>(linked.major) << "."
                << static_cast<int>(linked.minor) << "."
                << static_cast<int>(linked.patch) << endl;
  }

  XML::XPathProcessor config(config_xml);

  reconfigure();
  return true;
}

//--------------------------------------------------------------------------
// Global pre-configuration - called at startup
int Server::preconfigure()
{
  return 0;
}

//--------------------------------------------------------------------------
// Global re-configuration - called at SIGHUP
void Server::reconfigure()
{
  Log::Streams log;
  log.summary << "Configuring server dynamic state" << endl;

  // Shutdown any existing graph
  engine.shutdown();

  // Get number of threads
  unsigned threads = config_xml.get_child("thread").get_attr_int("count", 0);
  engine.set_threads(threads);

  // Get tick interval from frequency
  double freq = config_xml.get_child("tick").get_attr_real("frequency",
                                           Dataflow::default_frequency);
  if (freq > 0)
    engine.set_tick_interval(Time::Duration(1/freq));
  else
    engine.set_tick_interval(Time::Duration(1/Dataflow::default_frequency));

  // (Re)load modules
  const XML::Element& modules_e = config_xml.get_child("modules");
  for(const auto dir_e: modules_e.get_children("directory"))
  {
    const auto d = File::Directory{(*dir_e)["path"]};
    const auto d_exp = d.expand();
    const auto dir = File::Directory{config_file.resolve(d_exp)};
    if (dir.is_dir())
    {
      log.summary << "Searching directory " << dir << " for modules\n";
      list<File::Path> paths;
#if defined(PLATFORM_WINDOWS)
      dir.inspect_recursive(paths, "*.dll");
#else
      dir.inspect_recursive(paths, "*.so");
#endif
      for(const auto& p: paths) load_module(p);
    }
  }

  // Resource path
  const auto& resources_e = config_xml.get_child("resources");
  const auto& resource_dir_e = resources_e.get_child("directory");
  const auto r_d = File::Directory{resource_dir_e["path"]};
  const auto r_d_exp = r_d.expand();
  const auto r_dir = File::Directory{config_file.resolve(r_d_exp)};
  engine.set_resource_dir(r_dir);

  const auto& graph_e = config_xml.get_child("graph");
  const auto p = graph_e["file"];
  if (!p.empty())
  {
    graph_file = config_file.resolve(p);
    if (graph_file.exists())
      load_graph(graph_file);
    else
      log.error << "Graph file not found: " << graph_file << endl;
  }

  // (re-)create the REST interface
  rest.reset();
  const XML::Element& rest_e = config_xml.get_child("rest");
  if (!!rest_e) rest.reset(new RESTInterface(rest_e, engine, *this,
                                             config_file.dirname()));

  // (re-)create the file server
  file_server.reset();
  const XML::Element& file_server_e = config_xml.get_child("file-server");
  if (!!file_server_e) file_server.reset(new FileServer(file_server_e,
                                                        config_file.dirname()));
}

//--------------------------------------------------------------------------
// Load a graph file
bool Server::load_graph(const File::Path& path)
{
  Log::Streams log;

  graph_file_time = path.last_modified();

  const auto ext = path.extension();

  JSON::Value json;
  if (Text::tolower(ext) == "json")
  {
    try
    {
      auto s = string{};
      path.read_all(s);
      auto iss = istringstream{s};
      ObTools::JSON::Parser parser(iss);
      json = parser.read_value();
    }
    catch (ObTools::JSON::Exception& e)
    {
      log.error << "Graph load JSON parsing failed: " << e.error << endl;
      return false;
    }
  }
  else if (Text::tolower(ext) == "vg")
  {
    try
    {
      auto s = string{};
      path.read_all(s);
      auto iss = istringstream{s};
      Compiler::Parser parser(iss);
      parser.set_default_section(default_section);
      json = parser.get_elements_json();
    }
    catch (Compiler::Exception e)
    {
      log.error << "Graph load compile failed: " << e.error << endl;
      return false;
    }
  }
  else
  {
    log.error << "Unhandled file type for graph file: " << path << endl;
    graph_file_time = 0;
    return false;
  }

  try
  {
    JSON::set(engine, json, Dataflow::Path{""});
  }
  catch (runtime_error& e)
  {
    log.error << "Could not create graph from file: " << e.what() << endl;
    return false;
  }

  return true;
}

//--------------------------------------------------------------------------
// Load and initialise a module
bool Server::load_module(const File::Path& path)
{
  Log::Streams log;

  // Do we already have it, and it hasn't changed?
  Time::Stamp mtime(path.last_modified());
  auto it = modules.find(path.str());
  if (it != modules.end())
  {
    if (it->second.mtime == mtime)
    {
      log.detail << "Module " << path
                 << " already loaded and not changed - skipping\n";
      return true;
    }

    // Unload the old one, and erase
    modules.erase(it);
  }

  log.detail << "Loading module " << path << endl;
  auto mod = make_unique<Lib::Library>(path.str());
  if (!*mod)
  {
    log.error << "Can't open dynamic library " << path << ": "
              << mod->get_error() << endl;
    return false;
  }

  auto fn = mod->get_function<vg_init_fn_t *>("vg_init");
  if (!fn)
  {
    log.error << "No 'vg_init' symbol in dynamic library " << path << endl;
    return false;
  }

  // Pass our logger in, so the module can connect its own, and the registries
  if (!fn(Log::logger, engine))
  {
    log.error << "Module " << path << " initialisation failed\n";
    return false;
  }

  // Remember it, with the file's mtime
  modules[path.str()] = Module(mod.release(), mtime);
  return true;
}

//--------------------------------------------------------------------------
// Clean up
void Server::cleanup()
{
  Log::Summary log;
  log << "Shutting down\n";

  log << "Shutting down file server\n";
  file_server.reset();

  log << "Shutting down REST server\n";
  rest.reset();

  log << "Shutting down dataflow graph\n";
  engine.shutdown();

  log << "Unloading modules\n";
  modules.clear();

  log << "Shutting down SDL\n";
  SDL_Quit();

  log << "Shutdown complete\n";
}

//--------------------------------------------------------------------------
// Destructor
Server::~Server()
{
}

}} // namespaces
