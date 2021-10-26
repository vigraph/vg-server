//==========================================================================
// Internal definitions for communications / queue server
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_NEXUS_H
#define __VIGRAPH_NEXUS_H

#include "ot-daemon.h"

namespace ViGraph { namespace Nexus {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ObTools;
using namespace ViGraph;

//==========================================================================
// Global state
// Singleton instance of server-wide state
class Server: public Daemon::Application
{
public:
  //------------------------------------------------------------------------
  // Constructor
  Server() {}

  //------------------------------------------------------------------------
  // Time to sleep until next tick (microseconds)
  int tick_wait() override { return 100; }

  //------------------------------------------------------------------------
  // Read settings from configuration
  void read_config(const XML::Configuration& /*config*/,
                   const string& /*config_filename*/) override
  {
  }

  //------------------------------------------------------------------------
  // Prerun function for child process
  int run_priv() override
  {
    return 0;
  }

  //------------------------------------------------------------------------
  // Pre main loop function
  int pre_run() override
  {
    return 0;
  }

  //------------------------------------------------------------------------
  // Main loop iteration function
  int tick() override
  {
    return 0;
  }

  //------------------------------------------------------------------------
  // Global configuration - called only at startup
  // Returns whether successful
  bool configure()
  {
    return 0;
  }

  //------------------------------------------------------------------------
  // Global pre-configuration - called at startup
  int preconfigure() override
  {
    return 0;
  }

  //------------------------------------------------------------------------
  // Global re-configuration - called at SIGHUP
  void reconfigure() override
  {
  }

  //------------------------------------------------------------------------
  // Clean up
  void cleanup() override
  {
  }

  //------------------------------------------------------------------------
  // Virtual Destructor
  ~Server() {}
};

//==========================================================================
}} //namespaces
#endif // !__VIGRAPH_NEXUS_H
