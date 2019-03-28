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
class WaitControl: public Control
{
public:
  // Configured state
  double delay = 0.0;

private:
  Dataflow::timestamp_t delayed_until = -1.0;

  // Dynamic state
  bool triggered = false;

  // Control virtuals
  void pre_tick(const TickData& td) override;
  void enable() override;

public:
  using Control::Control;

  // Trigger
  void set_triggered() { triggered = true; }
};

//--------------------------------------------------------------------------
// Enable (reset)
void WaitControl::enable()
{
  delayed_until = -1.0;
}

//--------------------------------------------------------------------------
// Tick
void WaitControl::pre_tick(const TickData& td)
{
  // Apply delay if necessary
  if (triggered)
  {
    delayed_until = td.t + delay;
    triggered = false;
    return;
  }

  if (delayed_until > td.t)
    return;
  delayed_until = -1.0;

  // We're good - do it...
  trigger();
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "wait",
  "Wait",
  "Wait another Element",
  "core",
  {
    { "for", { "Time to wait for", Value::Type::number,
               &WaitControl::delay, true } },
    { "trigger", { "Trigger", Value::Type::trigger,
                   &WaitControl::set_triggered, true } }
  },
  { { "trigger", { "Trigger", "trigger", Value::Type::trigger }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(WaitControl, module)
