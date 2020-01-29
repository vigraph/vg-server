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
  void setup(const SetupContext& context) override;
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
  Setting<Integer> host_port{ArtNet::udp_port};
  Setting<string> source_address{default_source_address};
  Setting<Number> frame_rate{default_frame_rate};

  // Input
  Input<DMX::State> input;
};

//--------------------------------------------------------------------------
// Setup after config
void ArtNetOut::setup(const SetupContext& context)
{
  SimpleElement::setup(context);

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
  sample_iterate(td, nsamples, {}, tie(input), {},
                 [&](const DMX::State& input)
  {
    // Each region is a separate packet, maybe spread over multiple
    // universes
    for(const auto& rit: input.regions)
    {
      auto start_chan = rit.first;
      // Using the basic packet we have to start on an universe boundary
      // so we may have to pad the first packet
      auto first_padding = start_chan % DMX::channels_per_universe;

      const auto& values = rit.second;
      for(auto u=0; ;u++)
      {
        auto& sequence = universe_sequences[u];
        if (!sequence) sequence++;  // Avoid 0
        ArtNet::DMXPacket dmx_packet(sequence++, u);

        auto padding = u ? 0 : first_padding;
        auto copy_start = u*DMX::channels_per_universe+padding-first_padding;
        auto packet_length = min(DMX::channels_per_universe,
                                 values.size()-copy_start+padding);
        auto copy_length = min(DMX::channels_per_universe-padding,
                               values.size()-copy_start);
        dmx_packet.data.resize(packet_length, 0);
        copy(values.begin()+copy_start,
             values.begin()+copy_start+copy_length,
             dmx_packet.data.begin()+padding);
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

        if (copy_start + copy_length >= values.size()) break;
      }
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
    { "source-address",  &ArtNetOut::source_address    },
    { "frame-rate",      &ArtNetOut::frame_rate        }
  },
  {
    { "input",           &ArtNetOut::input }
  },
  {}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ArtNetOut, module)

