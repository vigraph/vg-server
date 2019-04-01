//==========================================================================
// ViGraph dataflow module: controls/pool-send/pool-send.cc
//
// Control to pool-send a control on/off
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include "../../../core/core-services.h"

using namespace ViGraph::Module::Core;

namespace {

//==========================================================================
// PoolSend control
class PoolSendControl: public Dataflow::Control
{
  shared_ptr<PoolDistributor> distributor;
  unsigned index = 0;

  // Control/Element virtuals
  void setup() override;

public:
  string pool;

  // Construct
  using Control::Control;

  // Getters/Setters
  void set_number(double number);
  void set_velocity(double velocity);
  void on();
  void off();
};

//--------------------------------------------------------------------------
// Setup
void PoolSendControl::setup()
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
// Set number
void PoolSendControl::set_number(double number)
{
  index = number;
  if (distributor)
    distributor->send(pool, index, "number", {number});
}

//--------------------------------------------------------------------------
// Set velocity
void PoolSendControl::set_velocity(double velocity)
{
  if (distributor)
    distributor->send(pool, index, "velocity", {velocity});
}

//--------------------------------------------------------------------------
// Key on
void PoolSendControl::on()
{
  if (distributor)
    distributor->send(pool, index, "trigger", {});
}

//--------------------------------------------------------------------------
// Key on
void PoolSendControl::off()
{
  if (distributor)
    distributor->send(pool, index, "clear", {});
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "pool-send",
  "Pool Send",
  "Send MIDI controls to a pool",
  "core",
  {
    { "pool", { "Pool name", Value::Type::text,
                &PoolSendControl::pool, false }},
    { "number", { "Note number", Value::Type::number,
                  { &PoolSendControl::set_number }, true }},
    { "velocity", { "Velocity", Value::Type::number,
                    { &PoolSendControl::set_velocity }, true }},
    { "trigger", { "Trigger note on", Value::Type::trigger,
                   &PoolSendControl::on, true }},
    { "clear", { "Trigger note off", Value::Type::trigger,
                 &PoolSendControl::off, true }},
  },
  { }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(PoolSendControl, module)
