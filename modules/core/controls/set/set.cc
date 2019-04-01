//==========================================================================
// ViGraph dataflow module: controls/set/set.cc
//
// Control to set / alter properties on other elements
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Set control
class SetControl: public Control
{
  // Configured state
public:
  double value{0.0};
  bool wait{false};

  // Dynamic state
  bool done{false};
  bool triggered{false};

  // Control virtuals
  void pre_tick(const TickData& td) override;
  void notify_target_of(const string& property) override;
  void enable() override;

public:
  using Control::Control;

  // Trigger to set value
  void set_triggered() { triggered = true; }
};

//--------------------------------------------------------------------------
// Automatically set wait flag if we are the trigger target of something
void SetControl::notify_target_of(const string& property)
{
  if (property == "value")
    wait = true;
}

//--------------------------------------------------------------------------
// Enable (reset)
void SetControl::enable()
{
  triggered = done = false;
}

//--------------------------------------------------------------------------
// Tick
void SetControl::pre_tick(const TickData&)
{
  if (wait)
  {
    if (!triggered) return;
    triggered = false;
  }
  else
  {
    // Only run once
    if (done) return;
    done = true;
  }

  // We're good - do it...
  send(Value{value});
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "set",
  "Set",
  "Set a value on another Element",
  "core",
  {
    { "value", { "Value to set", Value::Type::number,
                 &SetControl::value, true } },
    { "wait",  { "Whether to wait for a trigger", Value::Type::boolean,
                 &SetControl::wait, false } },
    { "trigger", { "Trigger to set value", Value::Type::trigger,
                   &SetControl::set_triggered, true } }
  },
  { { "value", { "Value", "value", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SetControl, module)
