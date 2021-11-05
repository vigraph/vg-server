//==========================================================================
// ViGraph Nexus server: server.cc
//
// Singleton server object
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#include "nexus.h"

namespace ViGraph { namespace Nexus {

const auto default_active_time{"1 min"};

//--------------------------------------------------------------------------
// Read settings from configuration
void Server::read_config(const XML::Configuration& _config,
                         const string& config_filename)
{
  config_xml = _config.get_root();
  config_file = File::Path(config_filename);
}

//--------------------------------------------------------------------------
// Main setup function
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
  Log::Streams log;

  // Check if the currently active client has timed out
  auto new_active_client_id = client_queue.check_time_up(Time::Stamp::now());
  if (new_active_client_id != current_active_client_id)
  {
    // !!! Send timeout message to the old one (if any)

    current_active_client_id = new_active_client_id;
    if (current_active_client_id.empty())
      log.summary << "Gone idle\n";
    else
      log.summary << "New active client: " << current_active_client_id << endl;
  }

  // !!! Send active messages to the active one (if any)

  // !!! Broadcast queue info messages

  return 0;
}

//--------------------------------------------------------------------------
// Global configuration - called only at startup
// Returns whether successful
bool Server::configure()
{
  XML::XPathProcessor config(config_xml);
  Log::Streams log;
  log.summary << "Configuring server permanent state" << endl;

  // Configure the client queue
  Time::Duration active_time(config.get_value("queue/active/@time",
                                              default_active_time));
  log.detail << "Active time is " << active_time.seconds() << "s\n";
  client_queue.set_active_time(active_time);

  // Create Web server
  auto port = config.get_value_int("http/server/@port");
  if (port)
  {
    log.summary << "Creating UI HTTP server on port " << port << endl;
    auto jwt_secret = config["http/jwt/@secret"];
    if (!jwt_secret.empty())
      log.summary << " - requiring JWT authentication\n";

    http_server.reset(new HTTPServer(nullptr /* !!! ssl_ctx.get()*/, port,
                                     client_queue, jwt_secret));

    // Background thread to run it
    http_server_thread.reset(new Net::TCPServerThread(*http_server));
    http_server_thread->detach();
  }

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
}

//--------------------------------------------------------------------------
// Clean up
void Server::cleanup()
{
  Log::Summary log;
  log << "Shutting down\n";

  log << "Shutdown complete\n";
}

}} // namespaces
