//==========================================================================
// ViGraph dataflow engine server: server.cc
//
// Singleton server object for engine server
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "engine.h"
#include "vg-licence.h"
#include <SDL.h>

namespace ViGraph { namespace Engine {

//--------------------------------------------------------------------------
// Constructor
Server::Server(const string& _licence_file): licence_file{_licence_file}
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
  Log::Streams log;

  // Assume the worst
  auto licenced = false;

  if (licence_file.size())
  {
    // Check licence first so logging is at front
    log.detail << "Reading licence from " << licence_file << endl;
    Licence::File licence_xml(licence_file, log.error);

    if (licence_xml.check())
    {
      if (licence_xml["licensee"].size())
        log.summary << "Licensed to " << licence_xml["licensee"] << endl;
      if (licence_xml["purpose"].size())
        log.summary << licence_xml["purpose"] << endl;

      // Get engine licence element
      const auto *licence_e = licence_xml.get_component("vg-engine");
      if (licence_e)
      {
        // ! read specific parameters when required
        licenced = true;
      }
    }
  }

  if (!licenced)
  {
    log.error
      << "===========================================================" << endl
      << "                No valid licence found" << endl
      << "    Please contact info@vigraph.com for full licensing" << endl
      << "===========================================================" << endl;
    return 666;
  }
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
  Time::Stamp now = Time::Stamp::now();
  engine.tick(now);
  return 0;
}

//--------------------------------------------------------------------------
// Global configuration - called only at startup
// Returns whether successful
bool Server::configure()
{
  Log::Streams log;
  log.summary << "Configuring server permanent state" << endl;

  // !!! Crashes on Ubuntu18 - removed to get Tom working
#if 0
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
  SDL_Init(SDL_INIT_EVERYTHING);
#endif

  XML::XPathProcessor config(config_xml);

  reconfigure();
  return true;
}

//--------------------------------------------------------------------------
// Global pre-configuration - called at startup
int Server::preconfigure()
{
  XML::XPathProcessor xpath(config_xml);
  licence_file = xpath.get_value("licence/@file", licence_file);
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

  // Get tick interval from frequency
  double freq = config_xml.get_child("tick").get_attr_real("frequency",
                                           Dataflow::default_frequency);
  if (freq > 0)
    engine.set_tick_interval(Time::Duration(1/freq));
  else
    engine.set_tick_interval(Time::Duration(1/Dataflow::default_frequency));

  // Get sample rate
  double sample_rate = config_xml.get_child("sample").get_attr_real("rate",
                                             Dataflow::default_sample_rate);
  engine.set_sample_rate(sample_rate);

  // (Re)load modules
  const XML::Element& modules_e = config_xml.get_child("modules");
  for(const auto dir_e: modules_e.get_children("directory"))
  {
    File::Directory dir((*dir_e)["path"]);
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

  // Set default sections
  const XML::Element& naming_e = config_xml.get_child("naming");
  for(const auto def_e: naming_e.get_children("default"))
  {
    const auto& section = (*def_e)["section"];
    log.summary << "Using default section name '" << section << "'\n";
    engine.add_default_section(section);
  }

  // Load graph from config <graph>
  const XML::Element& graph_e = config_xml.get_child("graph");
  if (!graph_e)
  {
    log.error << "No <graph> element in config\n";
    return;
  }

  // Configure the graph engine using XML
  try
  {
    // Based relative to our config filename
    engine.configure(config_file.dirname(), graph_e);
    log.summary << "Dataflow engine loaded OK\n";
  }
  catch (runtime_error& e)
  {
    log.error << "Can't create graph: " << e.what() << endl;
  }

  // (re-)create the REST interface
  rest.reset();
  const XML::Element& rest_e = config_xml.get_child("rest");
  if (!!rest_e) rest.reset(new RESTInterface(rest_e, engine));
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
