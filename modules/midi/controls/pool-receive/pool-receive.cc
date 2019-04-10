//==========================================================================
// ViGraph dataflow module: controls/pool-receive/pool-receive.cc
//
// Control to pool-receive a control on/off
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include "../../../core/core-services.h"

using namespace ViGraph::Module::Core;

namespace {

//==========================================================================
// PoolReceive control
class PoolReceiveControl: public Dataflow::Control
{
  shared_ptr<PoolDistributor> distributor;
  string pool;

  // Control/Element virtuals
  void set_property(const string& property, const Value& v) override;
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void enable() override;
  void disable() override;

public:
  // Construct
  PoolReceiveControl(const Dataflow::Module *module,
                     const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//   <pool-receive pool="x"/>
PoolReceiveControl::PoolReceiveControl(const Dataflow::Module *module,
                                       const XML::Element& config):
  Control(module, config)
{
  pool = config["pool"];
}

//--------------------------------------------------------------------------
// Configure from XML
void PoolReceiveControl::configure(const File::Directory&,
                                   const XML::Element&)
{
  distributor = graph->find_service<PoolDistributor>("pool-distributor");
}

//--------------------------------------------------------------------------
// Enable - register for events
void PoolReceiveControl::enable()
{
  if (distributor)
    distributor->register_worker(pool, this);
}

//--------------------------------------------------------------------------
// Disable - deregister for events
void PoolReceiveControl::disable()
{
  if (distributor)
    distributor->deregister_worker(this);
}

//--------------------------------------------------------------------------
// Set a control property
void PoolReceiveControl::set_property(const string& prop,
                                      const Value& v)
{
  send(prop, v);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "pool-receive",
  "PoolReceive",
  "PoolReceive a control on/off",
  "core",
  { },
  {
    { "number", { "Note number", "number", Value::Type::number }},
    { "velocity", { "Velocity", "velocity", Value::Type::number }},
    { "trigger", { "Note trigger", "trigger", Value::Type::trigger }},
    { "clear", { "Note clear", "clear", Value::Type::trigger }},
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(PoolReceiveControl, module)
