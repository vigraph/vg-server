//==========================================================================
// ViGraph dataflow module: dmx/ola-out/ola-out.cc
//
// DMX output through OLA
//
// Copyright (c) 2019-2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../dmx-module.h"
#include "vg-dmx.h"
#include <ola/client/ClientWrapper.h>

namespace {

using namespace ViGraph::Dataflow;
const auto default_frame_rate = 25;

//==========================================================================
// OLAOut module
class OLAOut: public SimpleElement
{
private:
  ola::client::OlaClientWrapper ola_client;
  struct BufferInfo
  {
    bool modified = false;
    ola::DmxBuffer buffer;
  };
  map<unsigned, BufferInfo> buffers;

  // Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;
  void shutdown() override;

  // Clone
  OLAOut *create_clone() const override
  {
    return new OLAOut{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<Number> frame_rate{default_frame_rate};

  // Input
  Input<DMX::State> input;
};

//--------------------------------------------------------------------------
// Setup
void OLAOut::setup(const SetupContext& context)
{
  SimpleElement::setup(context);
  Log::Streams log;

  input.set_sample_rate(frame_rate);

  // Input
  log.summary << "Starting OLA client\n";

  if (!ola_client.Setup())
  {
    log.error << "Couldn't start OLA client" << endl;
    return;
  }
}

//--------------------------------------------------------------------------
// Tick data
void OLAOut::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(frame_rate);
  sample_iterate(td, nsamples, {}, tie(input), {},
                 [&](const DMX::State& )//input)
  {

  });
}

//--------------------------------------------------------------------------
// Shut down
void OLAOut::shutdown()
{
  Log::Detail log;
  log << "Shutting down DMX OLA output\n";
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "ola-out",
  "OLA DMX Output",
  "dmx",
  {
    { "frame-rate",      &OLAOut::frame_rate        }
  },
  {
    { "input",           &OLAOut::input }
  },
  {}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(OLAOut, module)

