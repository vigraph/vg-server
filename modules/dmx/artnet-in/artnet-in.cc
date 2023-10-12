//==========================================================================
// ViGraph dataflow module: dmx/artnet-in/artnet-in.cc
//
// Artnet (DMX over UDP) input
//
// Copyright (c) 2023 Paul Clark.  All rights reserved
//==========================================================================

#include "../dmx-module.h"
#include "vg-artnet.h"

namespace {

using namespace ViGraph::Dataflow;
const auto default_listen_address = "0.0.0.0";

class ArtNetInThread;

//==========================================================================
// ArtNetIn filter
class ArtNetIn: public SimpleElement
{
private:
  Net::EndPoint listen;

  // State
  unique_ptr<Net::UDPSocket> socket;
  unique_ptr<ArtNetInThread> thread;
  atomic<bool> running{false};
  MT::Mutex state_queue_mutex;
  queue<DMX::State> state_queue;

  friend class ArtNetInThread;
  void run();

  // Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;

  // Clone
  ArtNetIn *create_clone() const override
  {
    return new ArtNetIn{module};
  }

  void shutdown();

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<string> listen_address{default_listen_address};
  Setting<Integer> listen_port{ArtNet::udp_port};

  // Output
  Output<DMX::State> output;

  // Destructor
  ~ArtNetIn() { shutdown(); }
};

//==========================================================================
// ArtNetIn thread
class ArtNetInThread: public MT::Thread
{
private:
  ArtNetIn& in;

  void run() override
  { in.run(); }

public:
  ArtNetInThread(ArtNetIn& _in): in{_in} {}
};

//--------------------------------------------------------------------------
// Setup after config
void ArtNetIn::setup(const SetupContext& context)
{
  SimpleElement::setup(context);

  Log::Streams log;

  listen = Net::EndPoint(Net::IPAddress(listen_address), listen_port);
  log.summary << "Creating ArtNet receiver on " << listen << endl;

  // Create listener socket
  socket.reset(new Net::UDPSocket(listen, true, true));

  // Start background thread
  thread.reset(new ArtNetInThread(*this));
  running = true;
  thread->start();
}

//--------------------------------------------------------------------------
// Run background
void ArtNetIn::run()
{
  Log::Streams log;
  unsigned char buffer[65536];

  while (running && socket)
  {
    try
    {
      while (ssize_t len = socket->recv(buffer, sizeof(buffer)))
      {
        if (len > 0)
        {
          Channel::BlockReader reader(buffer, len);
          ArtNet::DMXPacket packet;
          packet.read(reader);

          if (!!packet)
          {
            DMX::State state;
            auto channel = DMX::channel_number(packet.port_address, 1);
            state.regions[channel] = packet.data;
            MT::Lock lock{state_queue_mutex};
            state_queue.push(state);
          }
        }
      }
    }
    catch (const Net::SocketError &error)
    {
      if (running) log.error << "ArtNetIn socket error: " << error << endl;
      break;
    }
  }
}

//--------------------------------------------------------------------------
// Tick data
void ArtNetIn::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, {}, tie(output),
                 [&](DMX::State& output)
  {
    MT::Lock lock(state_queue_mutex);
    if (!state_queue.empty())
    {
      output = state_queue.front();
      state_queue.pop();
    }
  });
}

//--------------------------------------------------------------------------
// Shut down
void ArtNetIn::shutdown()
{
  Log::Detail log;
  log << "Shutting down ArtNet receiver\n";
  running = false;
  if (socket)
  {
    socket->shutdown();
    socket->close();
  }
  socket.reset();
  if (thread) thread->join();
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "artnet-in",
  "ArtNet Input",
  "dmx",
  {
    { "address",         &ArtNetIn::listen_address      },
    { "port",            &ArtNetIn::listen_port         },
  },
  {},
  {
    { "output",          &ArtNetIn::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ArtNetIn, module)

