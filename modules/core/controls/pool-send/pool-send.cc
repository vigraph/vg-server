//==========================================================================
// ViGraph dataflow module: controls/pool-send/pool-send.cc
//
// Control to pool-send a control on/off
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include "../../core-services.h"

using namespace ViGraph::Module::Core;

namespace {

//==========================================================================
// PoolSend control
class PoolSendControl: public Dataflow::Control
{
  shared_ptr<PoolDistributor> distributor;
  string pool;

  // Control/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;

public:
  // Construct
  PoolSendControl(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//   <pool-send pool="x"/>
PoolSendControl::PoolSendControl(const Dataflow::Module *module,
                                 const XML::Element& config):
  Element(module, config), Control(module, config, true)
{
  pool = config["pool"];
}

//--------------------------------------------------------------------------
// Configure from XML (once we have the engine)
void PoolSendControl::configure(const File::Directory&, const XML::Element&)
{
  auto& engine = graph->get_engine();
  distributor = engine.get_service<PoolDistributor>("pool-distributor");
  if (!distributor)
  {
    Log::Error log;
    log << "No <pool-distributor> service loaded\n";
  }
}

//--------------------------------------------------------------------------
// Set a control property
void PoolSendControl::set_property(const string& prop,
                                   const SetParams& sp)
{
  if (distributor && (prop == "on" || prop == "off"))
    distributor->send(pool, prop, sp);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "pool-send",
  "PoolSend",
  "PoolSend a control on/off",
  "core",
  {
    { "on", { "Note on", Value::Type::number, "@on", true }},
    { "off", { "Note off", Value::Type::number, "@off", true }},
  },
  { }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(PoolSendControl, module)
