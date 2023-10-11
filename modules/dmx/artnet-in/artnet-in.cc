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

//==========================================================================
// ArtNetIn filter
class ArtNetIn: public SimpleElement
{
private:
  Net::EndPoint listen;

  // State
  unique_ptr<Net::UDPSocket> socket;

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

//--------------------------------------------------------------------------
// Setup after config
void ArtNetIn::setup(const SetupContext& context)
{
  SimpleElement::setup(context);

  Log::Streams log;

  listen = Net::EndPoint(Net::IPAddress(listen_address), listen_port);
  log.summary << "Creating ArtNet receiver on " << listen << endl;

  // Create listener socket
  socket.reset(new Net::UDPSocket(listen, true));
}

//--------------------------------------------------------------------------
// Tick data
void ArtNetIn::tick(const TickData& td)
{
  if (!socket) return;

  const auto sample_rate = output.get_sample_rate();
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, {}, tie(output),
                 [&](DMX::State& )//output)
  {
    // !!! Read packets from a background thread
    // post DMXState values like OlaIn does
  });
}

//--------------------------------------------------------------------------
// Shut down
void ArtNetIn::shutdown()
{
  Log::Detail log;
  log << "Shutting down ArtNet receiver\n";
  socket.reset();
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

