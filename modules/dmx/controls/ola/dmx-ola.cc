//==========================================================================
// ViGraph dataflow module: dmx/control/ola/dmx-ola.cc
//
// DMX interface using OLA API
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include "../../dmx-services.h"
#include <ola/client/ClientWrapper.h>

using namespace ViGraph::Module::DMX;

namespace {

//==========================================================================
// DMX interface implementation
class DMXInterface: public Dataflow::Control,
                    public Distributor::EventObserver
{
private:
  shared_ptr<Distributor> distributor;

  ola::client::OlaClientWrapper ola_client;
  map<unsigned, ola::DmxBuffer> buffers;

  // Control virtuals
  void setup() override;
  void pre_tick(const TickData& td) override;
  void shutdown() override;

  // Event observer implementation
  void handle(unsigned universe, unsigned channel, dmx_value_t value) override;

  // OLA callbacks
  void register_callback(const ola::client::Result& result);
  void dmx_callback(const ola::client::DMXMetadata& metadata,
                    const ola::DmxBuffer& data);

public:
  int universe = 0;

  // Construct
  using Control::Control;
};

//==========================================================================
// DMXInterface implementation

//--------------------------------------------------------------------------
// Setup
void DMXInterface::setup()
{
  Log::Streams log;
  distributor = graph->find_service<Distributor>("dmx:distributor");

  // Input
  log.summary << "Starting OLA client\n";

  if (!ola_client.Setup())
  {
    log.error << "Couldn't start OLA client" << endl;
    return;
  }

  auto client = ola_client.GetClient();
  client->SetDMXCallback(ola::NewCallback(this, &DMXInterface::dmx_callback));
  client->RegisterUniverse(universe, ola::client::REGISTER,
             ola::NewSingleCallback(this, &DMXInterface::register_callback));

  // Output
  distributor->register_event_observer(Direction::out, universe, universe,
                                       0, 512, this);
}

//--------------------------------------------------------------------------
// Register callback
void DMXInterface::register_callback(const ola::client::Result& result)
{
  if (!result.Success())
  {
    Log::Error log;
    log << "Failed to register universe: " << result.Error() << endl;
  }
}

//--------------------------------------------------------------------------
// DMX callback
void DMXInterface::dmx_callback(const ola::client::DMXMetadata& metadata,
                                const ola::DmxBuffer& data)
{
  if (distributor)
  {
    auto values = vector<dmx_value_t>(data.Size());
    auto len = data.Size();
    data.Get(&values[0], &len);
    distributor->handle_event(Direction::in, metadata.universe, values);
  }
}

//--------------------------------------------------------------------------
// Pre Tick
void DMXInterface::pre_tick(const TickData&)
{
  auto select_server = ola_client.GetSelectServer();
  if (select_server)
  {
    select_server->RunOnce();
  }
}

//--------------------------------------------------------------------------
// Handle event
void DMXInterface::handle(unsigned universe, unsigned channel,
                          dmx_value_t value)
{
  auto client = ola_client.GetClient();
  if (client)
  {
    auto& buffer = buffers[universe];
    buffer.SetChannel(channel - 1, value);
    client->SendDMX(universe, buffer, {});
  }
}

//--------------------------------------------------------------------------
// Shut down
void DMXInterface::shutdown()
{
  Log::Detail log;
  log << "Shutting down DMX OLA\n";
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "dmx",
  "DMX Interface",
  "DMX Interface for OLA",
  "dmx",
  {
    { "universe",  { "Universe", Value::Type::number,
                     &DMXInterface::universe, false} },
  },
  {}  // No controlled properties
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(DMXInterface, module)
