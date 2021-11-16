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
const auto default_update_interval{"1 sec"};

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
  auto now = Time::Stamp::now();
  auto new_active_client_id = client_queue.check_time_up(now);
  if (new_active_client_id != current_active_client_id)
  {
    // Send timeout message to the old one (if any)
    if (!current_active_client_id.empty())
    {
      JSON::Value timeup(JSON::Value::OBJECT);
      timeup.set("type", "timeup");
      http_server->send(current_active_client_id, timeup);
    }

    current_active_client_id = new_active_client_id;
    if (current_active_client_id.empty())
      log.summary << "Gone idle\n";
    else
      log.summary << "New active client: " << current_active_client_id << endl;
  }

  if (now - last_update_time >= update_interval)
  {
    auto time_remaining = client_queue.get_time_remaining(now);

    // Send active messages to the active one (if any)
    if (!current_active_client_id.empty())
    {
      JSON::Value active(JSON::Value::OBJECT);
      active.set("type", "active");
      active.set("time", time_remaining);
      http_server->send(current_active_client_id, active);
    }

    // Send queue info mehssages to all that are waiting
    auto waiting = client_queue.get_waiting();
    auto active_time = client_queue.get_active_time();
    int position = 1;
    for(const auto& client_id: waiting)
    {
      JSON::Value qinfo(JSON::Value::OBJECT);
      qinfo.set("type", "qinfo");
      qinfo.set("position", position);
      qinfo.set("total", waiting.size());
      qinfo.set("time", (position-1)*active_time.seconds()
                        + time_remaining);
      http_server->send(client_id, qinfo);
      position++;
    }

    last_update_time = now;
  }

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

  update_interval = Time::Duration(config.get_value("queue/update/@interval",
                                                    default_update_interval));
  log.detail << "Update interval is " << update_interval.seconds() << "s\n";

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

  if (!!http_server) http_server->shutdown();
  if (!!http_server_thread) http_server_thread->join();
  http_server_thread.reset();
  http_server.reset();

  log << "Shutdown complete\n";
}

}} // namespaces