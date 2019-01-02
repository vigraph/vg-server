//==========================================================================
// ViGraph dataflow module: ui/controls/websocket/websocket.cc
//
// Control from a client WebSocket
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include "../../ui-services.h"
#include "ot-web.h"

using namespace ViGraph::Module::UI;

namespace {

class WebSocketControl;  // forward

//==========================================================================
// WebSocket server
class WebSocketControlServer: public Web::SimpleHTTPServer
{
  WebSocketControl *control;

  //------------------------------------------------------------------------
  // Interface to handle upgraded web socket
  virtual void handle_websocket(const Web::HTTPMessage& request,
                                const SSL::ClientDetails& client,
                                SSL::TCPSocket& socket,
                                Net::TCPStream& stream);

public:
  //------------------------------------------------------------------------
  // Constructor - see ot-web.h SimpleHTTPServer()
  WebSocketControlServer(int port, const string& version,
                         WebSocketControl *_control);
};

//==========================================================================
// WebSocket control
class WebSocketControl: public Dataflow::Control
{
  unique_ptr<WebSocketControlServer> server;
  unique_ptr<Net::TCPServerThread> server_thread;
  shared_ptr<KeyDistributor> key_distributor;

  // Control virtuals
  void configure(const XML::Element& config) override;
  void shutdown() override;

public:
  // Construct
  WebSocketControl(const Dataflow::Module *module, const XML::Element& config);

  // Handle a key
  void handle_key(int code);
};

//==========================================================================
// WebSocket server implementation

//------------------------------------------------------------------------
// Constructor - see ot-web.h SimpleHTTPServer()
WebSocketControlServer::WebSocketControlServer(int port, const string& version,
                                               WebSocketControl *_control):
  // Only one thread, no backlog, 30 sec timeout
  Web::SimpleHTTPServer(port, version, 0, 0, 1, 30), control(_control)
{
  // Allow cross-origin fetch from anywhere
  set_cors_origin();

  // Enable WebSocket
  enable_websocket();
}

//------------------------------------------------------------------------
// Interface to handle upgraded web socket
void WebSocketControlServer::handle_websocket(
                      const Web::HTTPMessage& /* request */,
                      const SSL::ClientDetails& /* client */,
                      SSL::TCPSocket& /* socket */,
                      Net::TCPStream& stream)
{
  Log::Streams log;
  log.detail << "Handling WebSocket control protocol\n";

  Web::WebSocketServer ws(stream);
  for(;;)
  {
    string msg;
    if (!ws.read(msg))
    {
      log.detail << "WebSocket control connection closed\n";
      return;
    }

    log.detail << "Received WebSocket control message, " << msg.size()
               << " bytes\n";

    Channel::StringReader sr(msg);
    try
    {
      int version = sr.read_byte();
      // v1 and v2 differ only by addition of key change
      if (version != 1 && version != 2)
      {
        log.error << "Bad WebSocket control protocol version: " << version
                  << endl;
        continue;
      }

      int type = sr.read_byte();
      switch (type)
      {
        case 2:  // Control change
        {
          uint16_t index = sr.read_nbo_16();
          uint16_t value = sr.read_nbo_16();
          log.detail << "Control " << index << " value " << value << endl;
          // !!! Use index?
          control->send(Dataflow::Value(value));
        }
        break;

        case 3:  // Key change
        {
          int code = sr.read_byte();
          bool pressed = sr.read_byte() != 0;
          log.detail << "Key " << code << " " << (pressed?"pressed":"released")
                     << endl;
          // Negative indicates released
          control->handle_key(pressed?code:-code);
        }
        break;

        default:
          log.error << "Bad WebSocket control message type: " << type << endl;
      }
    }
    catch (Channel::Error)
    {
      log.error << "Truncated WebSocket control message\n";
    }
  }
}

//==========================================================================
// WebSocket control implementation

//--------------------------------------------------------------------------
// Construct from XML:
//   <websocket-control port="33382"/>
WebSocketControl::WebSocketControl(const Dataflow::Module *module,
                                   const XML::Element& config):
  Element(module, config), Control(module, config, true) // optional targets
{
  int hport = config.get_attr_int("port");
  if (hport)
  {
    Log::Summary log;
    log << "Starting WebSocket control server at port " << hport << endl;
    server.reset(new WebSocketControlServer(hport,
                                  "ViGraph laserd WebSocket control server",
                                            this));

    // Start threads
    server_thread.reset(new Net::TCPServerThread(*server));
    server_thread->detach();
  }
}

//--------------------------------------------------------------------------
// Configure from XML (once we have the engine)
void WebSocketControl::configure(const XML::Element&)
{
  auto& engine = graph->get_engine();
  key_distributor = engine.get_service<KeyDistributor>("key-distributor");
  if (!key_distributor)
  {
    Log::Error log;
    log << "No key-distributor service loaded\n";
  }
}

//--------------------------------------------------------------------------
// Handle a key press
void WebSocketControl::handle_key(int code)
{
  if (key_distributor)
    key_distributor->handle_key(code);
}

//--------------------------------------------------------------------------
// Shut down
void WebSocketControl::shutdown()
{
  Log::Detail log;
  log << "Shutting down WebSocket control server\n";

  if (server.get()) server->shutdown();
  server_thread.reset();
  server.reset();
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "websocket-control",
  "WebSocket Control",
  "WebSocket server for keyboard/control input",
  "ui",
  {
    { "port", { "Listening port", Value::Type::number } },
  },
  { { "", { "Control index", "index", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(WebSocketControl, module)
