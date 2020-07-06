//==========================================================================
// ViGraph dataflow module: laser/etherdream-out/etherdream-out.cc
//
// Filter to output display to an EtherDream device (TCP)
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector/vector-module.h"
#include "vg-etherdream.h"

namespace {

using namespace ViGraph::Dataflow;
const auto default_source_address = "0.0.0.0";
const auto default_frame_rate = 50;

//==========================================================================
// EtherDreamOut filter
class EtherDreamOut: public SimpleElement
{
private:
  Net::EndPoint source, destination;

  // State
  unique_ptr<Net::TCPClient> socket;

  // Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;

  // Clone
  EtherDreamOut *create_clone() const override
  {
    return new EtherDreamOut{module};
  }

  void shutdown();

  // Internal
  void transmit(const Frame& frame, timestamp_t timestamp, double sample_rate);

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<string> host_address;
  Setting<Integer> host_port{EtherDream::default_port};
  Setting<string> source_address{default_source_address};
  Setting<Integer> frame_rate{default_frame_rate};

  // Input
  Input<Frame> input;

  // Destructor
  ~EtherDreamOut() { shutdown(); }
};

//--------------------------------------------------------------------------
// Setup after config
void EtherDreamOut::setup(const SetupContext& context)
{
  SimpleElement::setup(context);

  Log::Streams log;

  destination = Net::EndPoint(Net::IPAddress(host_address), host_port);
  log.summary << "Creating EtherDream transmitter to " << destination << endl;

  source = Net::EndPoint(Net::IPAddress(source_address), 0);
  // Bind to local port
  socket.reset(new Net::TCPClient(source, destination));

  source = socket->local();
  log.detail << "EtherDream transmitter bound to local address "
             << source << endl;

  input.set_sample_rate(frame_rate);
}

//--------------------------------------------------------------------------
// Tick data
void EtherDreamOut::tick(const TickData& td)
{
  const auto sample_rate = frame_rate;
  auto sample_time = td.first_sample_at(sample_rate);
  const auto sample_duration = td.sample_duration(sample_rate);
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, tie(input), {},
                 [&](const Frame& input)
  {
    transmit(input, sample_time, sample_rate);
    sample_time += sample_duration;
  });
}

//--------------------------------------------------------------------------
// Transmit a frame
void EtherDreamOut::transmit(const Frame& /*frame*/,
                             timestamp_t /*timestamp*/,
                             double /*sample_rate*/)
{
  if (!socket) return;
}

//--------------------------------------------------------------------------
// Shut down
void EtherDreamOut::shutdown()
{
  Log::Detail log;
  log << "Shutting down EtherDream transmitter\n";
  socket.reset();
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "etherdream-out",
  "EtherDream Output",
  "laser",
  {
    { "address",         &EtherDreamOut::host_address      },
    { "port",            &EtherDreamOut::host_port         },
    { "source-address",  &EtherDreamOut::source_address    },
    { "frame-rate",      &EtherDreamOut::frame_rate        }
  },
  {
    { "input",           &EtherDreamOut::input }
  },
  {}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(EtherDreamOut, module)

