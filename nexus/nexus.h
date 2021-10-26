//==========================================================================
// Internal definitions for communications / queue server
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_NEXUS_H
#define __VIGRAPH_NEXUS_H

#include "ot-daemon.h"
#include "ot-file.h"

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
  XML::Element config_xml;
  File::Path config_file;

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
                   const string& config_filename) override;

  //--------------------------------------------------------------------------
  // Main setup function
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
  ~Server() {}
};

//==========================================================================
}} //namespaces
#endif // !__VIGRAPH_NEXUS_H
