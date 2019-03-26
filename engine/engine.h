//==========================================================================
// ViGraph dataflow engine server: engine.h
//
// Internal definitions for dataflow engine server
//
// Copyright (c) 2017-2018 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_ENGINE_H
#define __VIGRAPH_ENGINE_H

#include "ot-daemon.h"
#include "ot-soap.h"
#include "vg-dataflow.h"
#include "vg-geometry.h"

namespace ViGraph { namespace Engine {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ObTools;
using namespace ViGraph;

// Module vg_init function interface
typedef bool vg_init_fn_t(Log::Channel&, Dataflow::Engine&);

//==========================================================================
// REST Interface
class RESTInterface
{
  unique_ptr<Web::SimpleHTTPServer> http_server;
  unique_ptr<Net::TCPServerThread> http_server_thread;

public:
  RESTInterface(const XML::Element& config, Dataflow::Engine& _engine);
  ~RESTInterface();
};

//==========================================================================
// Global state
// Singleton instance of server-wide state
class Server: public Daemon::Application
{
  XML::Element config_xml;
  File::Path config_file;
  string licence_file;

  // Loaded modules
  struct Module
  {
    void *dl_handle{0};
    Time::Stamp mtime;
    Module() {}
    Module(void *_h, const Time::Stamp& _t): dl_handle(_h), mtime(_t) {}
  };
  map <string, Module> modules;   // By pathname

  // Dataflow engine
  Dataflow::Engine engine;

  // Management interface
  unique_ptr<RESTInterface> rest;

  // Internal
  bool load_module(const File::Path& path);

public:
  //------------------------------------------------------------------------
  // Constructor
  Server(const string& _licence_file = "");

  //------------------------------------------------------------------------
  // Get the engine
  Dataflow::Engine& get_engine() { cout << "Get engine " << &engine << endl;
    return engine; }

  //------------------------------------------------------------------------
  // Time to sleep until next tick (microseconds)
  int tick_wait() override { return 100; }

  //------------------------------------------------------------------------
  // Read settings from configuration
  void read_config(const XML::Configuration& config,
                   const string& config_filename) override;

  //------------------------------------------------------------------------
  // Prerun function for child process
  int run_priv() override;

  //------------------------------------------------------------------------
  // Pre main loop function
  int pre_run() override;

  //------------------------------------------------------------------------
  // Main loop iteration function
  int tick() override;

  //------------------------------------------------------------------------
  // Global configuration - called only at startup
  // Returns whether successful
  bool configure();

  //------------------------------------------------------------------------
  // Global pre-configuration - called at startup
  int preconfigure() override;

  //------------------------------------------------------------------------
  // Global re-configuration - called at SIGHUP
  void reconfigure() override;

  //------------------------------------------------------------------------
  // Clean up
  void cleanup() override;

  //------------------------------------------------------------------------
  // Virtual Destructor
  ~Server();
};

//==========================================================================
}} //namespaces
#endif // !__VIGRAPH_ENGINE_H
