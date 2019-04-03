//==========================================================================
// ViGraph dataflow module: controls/set/set.cc
//
// Control to set / alter properties on other elements
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Set control
class TriggerControl: public Control
{
public:
  // Configured state
  bool wait{false};

private:
  // Dynamic state
  bool done{false};
  bool triggered{false};

  // Control virtuals
  void pre_tick(const TickData& td) override;
  void notify_target_of(const string& property) override;
  void enable() override;

public:
  using Control::Control;

  // Trigger
  void set_triggered() { triggered = true; }
};

//--------------------------------------------------------------------------
// Automatically set wait flag if we are the trigger target of something
void TriggerControl::notify_target_of(const string& property)
{
  if (property == "trigger")
    wait = true;
}

//--------------------------------------------------------------------------
// Enable (reset)
void TriggerControl::enable()
{
  triggered = done = false;
}

//--------------------------------------------------------------------------
// Tick
void TriggerControl::pre_tick(const TickData&)
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
  trigger();
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "trigger",
  "Trigger",
  "Trigger another Element",
  "core",
  {
    { "wait",  { "Whether to wait for a trigger", Value::Type::boolean,
                 &TriggerControl::wait, false } },
    { "trigger", { "Trigger", Value::Type::trigger,
                   &TriggerControl::set_triggered, true } }
  },
  { { "trigger", { "Trigger", "trigger", Value::Type::trigger }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(TriggerControl, module)
