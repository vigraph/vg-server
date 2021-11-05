//==========================================================================
// ViGraph Nexus server: server.cc
//
// Internal definitions for communications / queue server
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_NEXUS_H
#define __VIGRAPH_NEXUS_H

#include "ot-daemon.h"
#include "ot-file.h"
#include "ot-web.h"
#include "vg-nexus-queue.h"

namespace ViGraph { namespace Nexus {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ObTools;
using namespace ViGraph;

//==========================================================================
/// Queue entry
struct ClientEntry
{
  MT::Queue<string> msg_queue;
};

//==========================================================================
/// REST and Websocket server class
class HTTPServer: public ObTools::Web::SimpleHTTPServer
{
  // Registered clients
  MT::Mutex clients_mutex;
  map<string, ClientEntry *> clients;  // By ID

  // Client queue
  Queue& client_queue;

  // Secret for authentication - if empty, none is done
  string jwt_secret;

  // Interface to handle upgraded web socket
  virtual void handle_websocket(const Web::HTTPMessage& request,
                                const SSL::ClientDetails& client,
                                SSL::TCPSocket& socket,
                                Net::TCPStream& stream);

  // Override of auth check
  bool check_auth(const Web::HTTPMessage& request,
                  Web::HTTPMessage& response,
                  const SSL::ClientDetails& client);

public:
  /// Constructor - see ot-web.h SimpleHTTPServer()
  HTTPServer(ObTools::SSL::Context *ssl_ctx, int port,
             Queue& _client_queue,
             const string& _jwt_secret);

  /// Shutdown
  void shutdown();
};

//==========================================================================
// Global state
// Singleton instance of server-wide state
class Server: public Daemon::Application
{
  // Configuration read from file
  XML::Element config_xml;
  File::Path config_file;

  // HTTP server & thread
  std::unique_ptr<HTTPServer> http_server;
  std::unique_ptr<ObTools::Net::TCPServerThread> http_server_thread;

  // Client queue
  Queue client_queue;

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
