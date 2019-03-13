//==========================================================================
// ViGraph dataflow module: core/controls/wait/wait.cc
//
// Control to provide repetitive triggers at a defined interval
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>
#include <stdlib.h>

namespace {

//==========================================================================
// Wait control
class WaitControl: public Dataflow::Control
{
  // Configured state
  Time::Duration delay{0.0}; // 0 is next tick

  // Dynamic state
  bool triggered = false; // unless cancelled by trigger connection
  Dataflow::timestamp_t trigger_at = -1.0;

  // Reset state
  void reset();

  // Control virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void pre_tick(const TickData& td) override;
  void enable() override { reset(); }

public:
  // Construct
  WaitControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//    <wait delay="0.5"/>
WaitControl::WaitControl(const Module *module, const XML::Element& config):
  Control(module, config)
{
  if (config.has_attr("delay"))
    delay = Time::Duration{config["delay"]};
}

//--------------------------------------------------------------------------
// Reset state
void WaitControl::reset()
{
  triggered = false;
  trigger_at = -1.0;  // restart timebase
}

//--------------------------------------------------------------------------
// Set a control property
void WaitControl::set_property(const string& property, const SetParams&)
{
  if (property == "trigger")
  {
    triggered = true;
    trigger_at = -1.0;
  }
}

//--------------------------------------------------------------------------
// Tick
void WaitControl::pre_tick(const TickData& td)
{
  if (!triggered) return;

  if (triggered && trigger_at < 0)
  {
    trigger_at = td.t + delay.seconds();
    return;
  }
  if (td.t < trigger_at)
    return;

  reset();
  send(SetParams{Dataflow::Value{}});  // trigger
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "wait",
  "Wait",
  "Delay a trigger",
  "core",
  {
    { "delay", { "Delay in second", Value::Type::number, true } },
    { "trigger", { "Trigger to delay", Value::Type::trigger, true } },
  },
  { { "", { "Trigger output", "trigger", Value::Type::trigger }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(WaitControl, module)
