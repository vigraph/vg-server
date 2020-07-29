//==========================================================================
// Internal definitions for dataflow desktop application
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_DESKTOP_H
#define __VIGRAPH_DESKTOP_H

#include "vg-service.h"
#include "ot-daemon.h"
#include <QApplication>

namespace ViGraph { namespace Engine {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ObTools;
using namespace ViGraph;

//==========================================================================
// Desktop application mode
enum class Mode
{
  systray,
  full
};

//==========================================================================
// Global state
// Singleton instance of server-wide state
class Server: public Daemon::Application
{
private:
  Service::Server server;
  QApplication& app;
  MT::Semaphore& started;
  Mode mode = Mode::systray;

public:
  //------------------------------------------------------------------------
  // Constructor
  Server(QApplication& _app, MT::Semaphore& _started):
    app{_app}, started{_started} {}

  //------------------------------------------------------------------------
  // Time to sleep until next tick (microseconds)
  int tick_wait() override { return 100; }

  //------------------------------------------------------------------------
  // Read settings from configuration
  void read_config(const XML::Configuration& config,
                   const string& config_filename) override
  {
    server.read_config(config, config_filename);
    const auto xpath = XML::ConstXPathProcessor{config.get_root()};
    if (xpath.get_value("application/@mode", "systray") == "full")
      mode = Mode::full;
  }

  //------------------------------------------------------------------------
  // Get application mode
  Mode get_application_mode() const { return mode; }

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
    int result = server.pre_run();
    started.signal();
    return result;
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
    app.quit();
    return server.cleanup();
  }
};


//==========================================================================
}} //namespaces
#endif // !__VIGRAPH_DESKTOP_H
