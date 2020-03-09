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
  map<int, DMX::UniverseData> last_universes;

  // Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;
  void shutdown();

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

  ~OLAOut() { shutdown(); }
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
  auto client = ola_client.GetClient();
  if (!client) return;

  const auto nsamples = td.samples_in_tick(frame_rate);
  sample_iterate(td, nsamples, {}, tie(input), {},
                 [&](const DMX::State& input)
  {
    // Get a flat list of affected universes, padded with 0's
    map<int, DMX::UniverseData> universes;
    input.flatten(universes);

    // Send one buffer per universe
    for(const auto& uit: universes)
    {
      auto u = uit.first;
      const auto& ud = uit.second;
      auto& last_ud = last_universes[u];  // Will create 0 on first use
      if (ud == last_ud) continue;        // Don't send if not change
      last_ud = ud;

      ola::DmxBuffer buffer;
      buffer.Set(ud.channels.data(), ud.channels.size());
      client->SendDMX(u, buffer, {});
    }
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

