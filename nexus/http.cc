//==========================================================================
// ViGraph Nexus server: http.cc
//
// HTTP REST & WebSocket server for rules engine
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#include "nexus.h"

namespace ViGraph { namespace Nexus {

namespace
{
  const string server_ident{"ViGraph Nexus server"};
}

//==========================================================================
// Main web server

//------------------------------------------------------------------------
// Check Authorization JWT
bool HTTPServer::check_auth(const Web::HTTPMessage& request,
                            Web::HTTPMessage& /*response*/,
                            const ObTools::SSL::ClientDetails& /*client*/)
{
  // If no secret, leave it open
  if (jwt_secret.empty()) return true;

  // OPTIONS (CORS) is free
  if (request.method == "OPTIONS") return true;

  string token;

  // Check for Authorization: Bearer <jwt>
  const auto& authorization = request.headers.get("authorization");
  if (!authorization.empty())
  {
    auto bits = Text::split(authorization, ' ');
    if (bits.size() != 2 || Text::tolower(bits[0]) != "bearer")
    {
      Log::Error log;
      log << "Authorization header is not Bearer xxx\n";
      return false;
    }

    token = bits[1];
  }
  else
  {
    // Look for a token parameter
    token = request.url.get_query_parameter("token");
  }

  if (token.empty())
  {
    Log::Error log;
    log << "No authorization header or token provided\n";
    return false;
  }

  // Read JWT from it
  Web::JWT jwt(token);
  if (!jwt)
  {
    Log::Error log;
    log << "Can't parse Authorization JWT\n";
    return false;
  }

  // Verify signature
  if (!jwt.verify(jwt_secret)) return false;  // It'll do its own logging

  // Check expiry
  if (jwt.expired())
  {
    Log::Error log;
    log << "JWT has expired\n";
    return false;
  }

  return true;
}

//------------------------------------------------------------------------
// Constructor - see ot-web.h SimpleHTTPServer()
HTTPServer::HTTPServer(ObTools::SSL::Context *ssl_ctx, int port,
                       map<string, shared_ptr<Resource>>& _resources,
                       Time::Duration _active_time,
                       const string& _jwt_secret):
  // Max of 50, no pre-create, backlog 5, timeout 60
  Web::SimpleHTTPServer(ssl_ctx, port, server_ident, 5, 0, 50, 60),
  resources(_resources), active_time(_active_time),
  jwt_secret(_jwt_secret)
{
  // Allow cross-origin fetch from anywhere
  set_cors_origin();

  // Enable WebSocket
  enable_websocket();
}

//------------------------------------------------------------------------
// Ensure a resource exists
shared_ptr<Resource>& HTTPServer::ensure_resource(const string& resource_id)
{
  shared_ptr<Resource>& resource = resources[resource_id];
  if (!resource)
  {
    resource.reset(new Resource);
    resource->queue.set_active_time(active_time);
  }
  return resource;
}

//------------------------------------------------------------------------
// Interface to handle upgraded web socket
void HTTPServer::handle_websocket(const Web::HTTPMessage& /* request */,
                                  const ObTools::SSL::ClientDetails& client,
                                  ObTools::SSL::TCPSocket& /* socket */,
                                  Net::TCPStream& stream)
{
  Log::Streams log;
  log.detail << "Handling WebSocket UI protocol\n";

  Web::WebSocketServer ws(stream);

  // Register client message queue
  auto client_id = client.address.str();
  ClientEntry entry;
  {
    MT::Lock lock(clients_mutex);
    clients[client_id] = &entry;
  }

  log.summary << "Registered client " << client_id << endl;

  // Thread to read requests from the client
  atomic<bool> closed{false};
  thread read_thread{[this, &closed, &ws, &entry, &client_id, &log]()
  {
    string raw;
    string resource_id{"default"};

    // Loop while reading for valid messages, exit on failure
    while (ws.read(raw))
    {
      istringstream iss(raw);
      JSON::Parser parser(iss);
      JSON::Value json;
      try
      {
        json = parser.read_value();
      }
      catch (JSON::Exception e)
      {
        log.error << "Bad JSON: " << e.error << endl;
        break;
      }

      const auto& type = json["type"].as_str();
      if (type == "join")
      {
        resource_id = json["resource"].as_str("default");
        log.summary << "Client " << client_id << " joined queue on resource "
                    << resource_id << endl;
        auto& resource = ensure_resource(resource_id);
        resource->queue.add(client_id, Time::Stamp::now());
      }
      else if (type == "subscribe")
      {
        const auto& resource_id = json["resource"].as_str("default");
        log.summary << "Client " << client_id << " subscribed to resource "
                    << resource_id << endl;
        MT::Lock lock(clients_mutex);
        auto& resource = ensure_resource(resource_id);
        resource->subscribers[client_id] = &entry;
      }
      else if (type == "control")
      {
        // Reflect it to all subscribers on current resource
        if (!!resources[resource_id])
        {
          MT::Lock lock(clients_mutex);
          auto& resource = resources[resource_id];
          if (resource->queue.get_active() == client_id)
          {
            for(auto& p: resource->subscribers)
              p.second->msg_queue.send(raw);
          }
          else log.error << "Control received from non-head-of-queue: "
                         << client_id << endl;
        }
      }
      else
      {
        log.error << "Unrecognised message type '" << type << "'\n";
        break;
      }
    }

    closed = true;
    // Wake up the sender
    entry.msg_queue.send("");
  }};

  while (!closed)
  {
    const string& msg = entry.msg_queue.wait();
    if (msg.empty()) // Shutdown on empty marker
    {
      log.detail << "Shutting down WebSocket UI connection\n";
      ws.close();
      break;
    }

    // Send text message
    if (!ws.write(msg))
    {
      log.detail << "WebSocket connection ended\n";
      break;
    }
  }

  read_thread.join();

  // Deregister client
  log.summary << "De-registering client " << client_id << endl;
  {
    MT::Lock lock(clients_mutex);
    clients.erase(client_id);
    for(auto& p: resources)
    {
      if (!!p.second)
      {
        p.second->queue.remove(client_id);
        p.second->subscribers.erase(client_id);
      }
    }
  }

  log.detail << "WebSocket UI connection closed\n";
}

//------------------------------------------------------------------------
// Send a JSON message to a given client
void HTTPServer::send(const string& client_id, const JSON::Value& json)
{
  MT::Lock lock(clients_mutex);
  auto p = clients.find(client_id);
  if (p == clients.end()) return;

  auto client = p->second;
  client->msg_queue.send(json.str());
}

//------------------------------------------------------------------------
// Shut down
void HTTPServer::shutdown()
{
  // Send empty string to all client queues to force shutdown
  {
    MT::Lock lock(clients_mutex);
    for(auto& p: clients)
      p.second->msg_queue.send("");
  }

  Web::SimpleHTTPServer::shutdown();
}

}} // namespaces
