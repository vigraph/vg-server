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
                            const SSL::ClientDetails& /*client*/)
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
HTTPServer::HTTPServer(SSL::Context *ssl_ctx,
                       int port, const string& _jwt_secret):
  // Max of 50, no pre-create, backlog 5, timeout 60
  Web::SimpleHTTPServer(ssl_ctx, port, server_ident, 50, 0, 5, 60),
  jwt_secret(_jwt_secret)
{
  // Allow cross-origin fetch from anywhere
  set_cors_origin();

  // Enable WebSocket
  enable_websocket();
}

//------------------------------------------------------------------------
// Interface to handle upgraded web socket
void HTTPServer::handle_websocket(const Web::HTTPMessage& /* request */,
                                  const SSL::ClientDetails& /* client */,
                                  SSL::TCPSocket& /* socket */,
                                  Net::TCPStream& stream)
{
  Log::Streams log;
  log.detail << "Handling WebSocket UI protocol\n";

  Web::WebSocketServer ws(stream);

  // Register message queue for broadcasts
  MT::Queue<string> msg_queue;
  {
    MT::Lock lock(active_queue_mutex);
    active_queues.insert(&msg_queue);
  }

  // Thread to read requests from the client
  atomic<bool> closed{false};
  thread read_thread{[&closed, &ws, &msg_queue]()
  {
    string raw;
    // Loop while reading for valid messages, exit on failure
    while (ws.read(raw))
    {
      // Ignore for now!
    }

    closed = true;
    // Wake up the sender
    msg_queue.send("");
  }};

  while (!closed)
  {
    const string& msg = msg_queue.wait();
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

  // Deregister message queue for broadcasts
  {
    MT::Lock lock(active_queue_mutex);
    active_queues.erase(&msg_queue);
  }

  log.detail << "WebSocket UI connection closed\n";
}

//------------------------------------------------------------------------
// Shut down
void HTTPServer::shutdown()
{
  // Send empty string to all queues to force shutdown
  MT::Lock lock(active_queue_mutex);
  for(auto q: active_queues)
    q->send("");

  Web::SimpleHTTPServer::shutdown();
}

}} // namespaces
