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
  string pool;

  // Control/Element virtuals
  void set_property(const string& property, const Value& v) override;
  void calculate_topology(Element::Topology& topo) override;
  void enable() override;
  void disable() override;

public:
  // Construct
  using Control::Control;

  string get_pool() const { return pool; }
  void set_pool(const string& _pool);
};

//--------------------------------------------------------------------------
// Topology calculation - register as receiver
void PoolReceiveControl::calculate_topology(Element::Topology& topo)
{
  topo.router_receivers["pool:" + pool].push_back(this);
}

//--------------------------------------------------------------------------
// Enable - register for events
void PoolReceiveControl::enable()
{
  auto distributor =
    graph->find_service<PoolDistributor>("core", "pool-distributor");
  if (distributor)
    distributor->register_worker(pool, this);
}

//--------------------------------------------------------------------------
// Disable - deregister for events
void PoolReceiveControl::disable()
{
  auto distributor =
    graph->find_service<PoolDistributor>("core", "pool-distributor");
  if (distributor)
    distributor->deregister_worker(this);
}

//--------------------------------------------------------------------------
// Set pool
void PoolReceiveControl::set_pool(const string& _pool)
{
  if (_pool != pool)
  {
    auto distributor =
      graph->find_service<PoolDistributor>("core", "pool-distributor");
    if (distributor)
    {
      distributor->deregister_worker(this);
      distributor->register_worker(_pool, this);
    }
    pool = _pool;
  }
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
  "midi",
  {
    { "pool", { "Pool name", Value::Type::text,
                { &PoolReceiveControl::get_pool,
                  &PoolReceiveControl::set_pool }, true } },
  },
  {
    { "number", { "Note number", "number", Value::Type::number }},
    { "velocity", { "Velocity", "velocity", Value::Type::number }},
    { "trigger", { "Note trigger", "trigger", Value::Type::trigger }},
    { "clear", { "Note clear", "clear", Value::Type::trigger }},
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(PoolReceiveControl, module)
