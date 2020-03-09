//==========================================================================
// ViGraph dataflow module: dmx/ola-in/ola-in.cc
//
// DMX input from OLA
//
// Copyright (c) 2019-2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../dmx-module.h"
#include "vg-dmx.h"
#include <ola/client/ClientWrapper.h>

namespace {

using namespace ViGraph::Dataflow;

//==========================================================================
// OLAIn module
class OLAIn: public SimpleElement
{
private:
  ola::client::OlaClientWrapper ola_client;

  // Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;
  void shutdown();

  // Clone
  OLAIn *create_clone() const override
  {
    return new OLAIn{module};
  }

  // OLA callbacks
  void register_callback(const ola::client::Result& result);
  void dmx_callback(const ola::client::DMXMetadata& metadata,
                    const ola::DmxBuffer& data);

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<Integer> universe{0};

  // Output
  Output<DMX::State> output;

  ~OLAIn() { shutdown(); }
};

//--------------------------------------------------------------------------
// Setup
void OLAIn::setup(const SetupContext& context)
{
  SimpleElement::setup(context);
  Log::Streams log;

  // Input
  log.summary << "Starting OLA client\n";

  if (!ola_client.Setup())
  {
    log.error << "Couldn't start OLA client" << endl;
    return;
  }

  auto client = ola_client.GetClient();
  client->SetDMXCallback(ola::NewCallback(this, &OLAIn::dmx_callback));
  client->RegisterUniverse(universe, ola::client::REGISTER,
             ola::NewSingleCallback(this, &OLAIn::register_callback));
}

//--------------------------------------------------------------------------
// Register callback
void OLAIn::register_callback(const ola::client::Result& result)
{
  if (!result.Success())
  {
    Log::Error log;
    log << "Failed to register universe: " << result.Error() << endl;
  }
}

//--------------------------------------------------------------------------
// DMX callback
void OLAIn::dmx_callback(const ola::client::DMXMetadata&,// metadata,
                             const ola::DmxBuffer&)// data)
{
  //    auto values = vector<dmx_value_t>(data.Size());
  //  auto len = data.Size();
  //  data.Get(&values[0], &len);
  //    distributor->handle_event(Direction::in, metadata.universe, values);
}

//--------------------------------------------------------------------------
// Tick data
void OLAIn::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, {}, tie(output),
                 [&](DMX::State& )//output)
  {

  });
}

//--------------------------------------------------------------------------
// Shut down
void OLAIn::shutdown()
{
  Log::Detail log;
  log << "Shutting down DMX OLA input\n";
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "ola-in",
  "OLA DMX Input",
  "dmx",
  {
    { "universe",        &OLAIn::universe          }
  },
  {},
  {
    { "output",          &OLAIn::output }
  },
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(OLAIn, module)

