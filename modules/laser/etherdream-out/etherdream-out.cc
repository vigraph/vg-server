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
const auto default_frame_rate = 50;

//==========================================================================
// EtherDreamOut filter
class EtherDreamOut: public SimpleElement
{
private:
  Net::EndPoint destination;

  // State
  unique_ptr<EtherDream::TCPInterface> etherdream;

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
  Setting<Integer> frame_rate{default_frame_rate};
  Setting<Integer> point_rate{EtherDream::Interface::default_point_rate};

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

  etherdream.reset(new EtherDream::TCPInterface(destination, point_rate));
  etherdream->start();

  input.set_sample_rate(frame_rate);
}

//--------------------------------------------------------------------------
// Tick data
void EtherDreamOut::tick(const TickData& td)
{
  const auto sample_rate = frame_rate;
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, tie(input), {},
                 [&](const Frame& input)
  {
    if (!input.points.empty())
    {
      if (etherdream->get_ready())
      {
        auto available = etherdream->get_buffer_points_available();
        if (input.points.size() <= available)
        {
          etherdream->send(input.points);
        }
        else
        {
          Log::Error log;
          log << "Etherdream overflow - only " << available
              << " points available, " << input.points.size()
              << " to send - frame skipped\n";
        }
      }
    }
  });
}

//--------------------------------------------------------------------------
// Shut down
void EtherDreamOut::shutdown()
{
  Log::Detail log;
  log << "Shutting down EtherDream transmitter\n";
  etherdream.reset();
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
    { "frame-rate",      &EtherDreamOut::frame_rate        },
    { "point-rate",      &EtherDreamOut::point_rate        }
  },
  {
    { "input",           &EtherDreamOut::input }
  },
  {}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(EtherDreamOut, module)

