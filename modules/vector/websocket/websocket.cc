//==========================================================================
// ViGraph dataflow module: vector/websocket/websocket.cc
//
// Filter to output display to a client WebSocket
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include "ot-web.h"

namespace {

const auto default_port = 33382;
const auto default_frame_rate = 25;

//==========================================================================
// WebSocket display server

class WebSocketDisplayServer: public Web::SimpleHTTPServer
{
  struct FrameQueueEntry
  {
    Time::Stamp timestamp;
    FramePtr frame;          // nullptr = shutdown

    FrameQueueEntry() {}
    FrameQueueEntry(const Time::Stamp& t, FramePtr f):
      timestamp(t), frame(f) {}
  };
  MT::Queue<FrameQueueEntry> frame_queue; // !!! Assuming only one connection??
  int max_frame_queue_length{10};

  //------------------------------------------------------------------------
  // Interface to handle upgraded web socket
  virtual void handle_websocket(const Web::HTTPMessage& request,
                                const SSL::ClientDetails& client,
                                SSL::TCPSocket& socket,
                                Net::TCPStream& stream);

public:
  //------------------------------------------------------------------------
  // Constructor - see ot-web.h SimpleHTTPServer()
  WebSocketDisplayServer(int port, const string& version);

  //------------------------------------------------------------------------
  // Queue up a frame to be sent
  void queue(FramePtr frame)
  {
    frame_queue.limit(max_frame_queue_length);
    frame_queue.send(FrameQueueEntry(Time::Stamp::now(), frame));
  }
};

//------------------------------------------------------------------------
// Constructor - see ot-web.h SimpleHTTPServer()
WebSocketDisplayServer::WebSocketDisplayServer(int port, const string& version):
  // Only one thread, no backlog, 30 sec timeout
  Web::SimpleHTTPServer(port, version, 0, 0, 1, 30)
{
  // Allow cross-origin fetch from anywhere
  set_cors_origin();

  // Enable WebSocket
  enable_websocket();
}

//------------------------------------------------------------------------
// Interface to handle upgraded web socket
void WebSocketDisplayServer::handle_websocket(
                      const Web::HTTPMessage& /* request */,
                      const SSL::ClientDetails& /* client */,
                      SSL::TCPSocket& /* socket */,
                      Net::TCPStream& stream)
{
  Log::Streams log;
  log.detail << "Handling WebSocket display protocol\n";

  Web::WebSocketServer ws(stream);
  for(;;)
  {
    const FrameQueueEntry& fqe = frame_queue.wait();
    if (!fqe.frame) // Shutdown on 0 frame
    {
      log.detail << "Shutting down WebSocket display connection\n";
      ws.close();
      break;
    }

    // Construct realtime message
    string msg;
    Channel::StringWriter writer(msg);
    writer.write_byte(0x01);                   // Version
    writer.write_byte(0x01);                   // Vector frame
    writer.write_nbo_64(fqe.timestamp.ntp());  // Timestamp
    for(const auto& p: fqe.frame->points)
    {
      writer.write_nbo_16(static_cast<uint16_t>(p.x*65535+32768));
      writer.write_nbo_16(static_cast<uint16_t>(p.y*65535+32768));
      writer.write_nbo_16(static_cast<uint16_t>(p.c.r*65535));
      writer.write_nbo_16(static_cast<uint16_t>(p.c.g*65535));
      writer.write_nbo_16(static_cast<uint16_t>(p.c.b*65535));
    }

    if (!ws.write_binary(msg))
    {
      log.error << "WebSocket connection failed\n";
      break;
    }
  }
}

//==========================================================================
// WebSocket filter
class WebSocket: public SimpleElement
{
private:
  unique_ptr<WebSocketDisplayServer> server;
  unique_ptr<Net::TCPServerThread> server_thread;

  // Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;

  // Clone
  WebSocket *create_clone() const override
  {
    return new WebSocket{module};
  }

  void shutdown() override;

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<Integer> port{default_port};
  Setting<Number> frame_rate{default_frame_rate};

  // Input
  Input<Frame> input;
};

//--------------------------------------------------------------------------
// Setup
void WebSocket::setup(const SetupContext&)
{
  if (!!server) shutdown();

  if (port)
  {
    Log::Summary log;
    log << "Starting WebSocket display server at port " << port << endl;
    server.reset(new WebSocketDisplayServer(port,
                           "ViGraph WebSocket display server"));
    server_thread.reset(new Net::TCPServerThread(*server));

    input.set_sample_rate(frame_rate);
  }
}

//--------------------------------------------------------------------------
// Tick data
void WebSocket::tick(const TickData& td)
{
  const auto sample_rate = input.get_sample_rate();
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, tie(input), {},
                 [&](const Frame& input)
  {
    if (!!server)
    {
      FramePtr frame{new Frame{input}};
      server->queue(frame);
    }
  });
}

//--------------------------------------------------------------------------
// Shut down
void WebSocket::shutdown()
{
  Log::Detail log;
  log << "Shutting down WebSocket display server\n";

  // Send an empty frame to force shutdown of connection
  if (!!server)
  {
    server->queue(nullptr);
    server->shutdown();
  }
  if (server_thread)
    server_thread->join();
  server_thread.reset();
  server.reset();
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "websocket-display",
  "WebSocket Display",
  "vector",
  {
    { "port",  &WebSocket::port },
    { "frame-rate",  &WebSocket::frame_rate }
  },
  {
    { "input", &WebSocket::input }
  },
  {}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(WebSocket, module)
