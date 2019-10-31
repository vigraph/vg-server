//==========================================================================
// ViGraph dataflow module: dmx/artnet-out/artnet-out.cc
//
// Artnet (DMX over UDP) output
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../dmx-module.h"
#include "vg-artnet.h"

namespace {

using namespace ViGraph::Dataflow;
const auto default_source_address = "0.0.0.0";
const auto default_frame_rate = 25;
const auto default_universe{0};

//==========================================================================
// ArtNetOut filter
class ArtNetOut: public SimpleElement
{
private:
  Net::EndPoint source, destination;

  // State
  unique_ptr<Net::UDPSocket> socket;
  map<int, uint8_t> universe_sequences;

  // Element virtuals
  void setup() override;
  void tick(const TickData& td) override;

  // Clone
  ArtNetOut *create_clone() const override
  {
    return new ArtNetOut{module};
  }

  void shutdown() override;

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<string> host_address;
  Setting<int> host_port{ArtNet::udp_port};
  Setting<string> source_address{default_source_address};
  Setting<double> frame_rate{default_frame_rate};
  Setting<double> universe{default_universe};

  // Input
  Input<DMXState> input;
};

//--------------------------------------------------------------------------
// Setup after config
void ArtNetOut::setup()
{
  Log::Streams log;

  destination = Net::EndPoint(Net::IPAddress(host_address), host_port);
  log.summary << "Creating ArtNet transmitter to " << destination << endl;

  source = Net::EndPoint(Net::IPAddress(source_address), 0);
  // Bind to local port
  socket.reset(new Net::UDPSocket(source, true));

  source = socket->local();
  log.detail << "ArtNet transmitter bound to local address " << source << endl;

  input.set_sample_rate(frame_rate);
}

//--------------------------------------------------------------------------
// Tick data
void ArtNetOut::tick(const TickData& td)
{
  if (!socket) return;

  const auto nsamples = td.samples_in_tick(frame_rate);
  sample_iterate(nsamples, {}, tie(input), {},
                 [&](const DMXState& input)
  {
    // Can spread over multiple Universes
    for(int u=universe; ;u++)
    {
      auto& sequence = universe_sequences[u];
      if (!sequence) sequence++;  // Avoid 0
      ArtNet::DMXPacket dmx_packet(sequence++, u);

      auto start_chan = u*512ul;
      auto this_length = min(512ul, input.channels.size()-start_chan);
      dmx_packet.data.resize(this_length);
      copy(input.channels.begin()+start_chan,
           input.channels.begin()+start_chan+this_length,
           dmx_packet.data.begin());

      auto size = dmx_packet.length();

      // Create a packet
      vector<unsigned char> packet;
      packet.resize(size);
      Channel::BlockWriter bw(packet.data(), size);
      dmx_packet.write(bw);

      // Send it
      try
      {
        socket->sendto(packet.data(), size, 0, destination);
      }
      catch (Net::SocketError e)
      {
        Log::Error log;
        log << "ArtNet transmit socket error: " << e.get_string() << endl;
        return;
      }

      if (start_chan + this_length >= input.channels.size()) break;
    }
  });
}

//--------------------------------------------------------------------------
// Shut down
void ArtNetOut::shutdown()
{
  Log::Detail log;
  log << "Shutting down ArtNet transmit server\n";
  socket.reset();
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "artnet-out",
  "ArtNet Output",
  "dmx",
  {
    { "address",         &ArtNetOut::host_address      },
    { "port",            &ArtNetOut::host_port         },
    { "source=address",  &ArtNetOut::source_address    },
    { "frame-rate",      &ArtNetOut::frame_rate        },
    { "universe",        &ArtNetOut::universe          }
  },
  {
    { "input",           &ArtNetOut::input }
  },
  {}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ArtNetOut, module)

