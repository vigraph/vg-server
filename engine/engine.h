//==========================================================================
// Internal definitions for dataflow engine
//
// Copyright (c) 2017-2020 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_ENGINE_H
#define __VIGRAPH_ENGINE_H

#include "vg-service.h"
#include "ot-daemon.h"

namespace ViGraph { namespace Engine {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ObTools;
using namespace ViGraph;

//==========================================================================
// Global state
// Singleton instance of server-wide state
class Server: public Daemon::Application
{
private:
  Service::Server server;

public:
  //------------------------------------------------------------------------
  // Constructor
  Server() {}

  //------------------------------------------------------------------------
  // Time to sleep until next tick (microseconds)
  int tick_wait() override { return 100; }

  //------------------------------------------------------------------------
  // Read settings from configuration
  void read_config(const XML::Configuration& config,
                   const string& config_filename) override
  {
    server.read_config(config, config_filename);
  }

  //------------------------------------------------------------------------
  // Prerun function for child process
  int run_priv() override
  {
    return server.run_priv();
  }

  //------------------------------------------------------------------------
  // Pre main loop function
  int pre_run() override
  {
    return server.pre_run();
  }

  //------------------------------------------------------------------------
  // Main loop iteration function
  int tick() override
  {
    return server.tick();
  }

  //------------------------------------------------------------------------
  // Global configuration - called only at startup
  // Returns whether successful
  bool configure()
  {
    return server.configure();
  }

  //------------------------------------------------------------------------
  // Global pre-configuration - called at startup
  int preconfigure() override
  {
    return server.preconfigure();
  }

  //------------------------------------------------------------------------
  // Global re-configuration - called at SIGHUP
  void reconfigure() override
  {
    return server.reconfigure();
  }

  //------------------------------------------------------------------------
  // Clean up
  void cleanup() override
  {
    return server.cleanup();
  }

  //------------------------------------------------------------------------
  // Virtual Destructor
  ~Server() {}
};

//==========================================================================
}} //namespaces
#endif // !__VIGRAPH_ENGINE_H
