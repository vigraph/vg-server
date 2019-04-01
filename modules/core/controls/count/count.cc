//==========================================================================
// ViGraph dataflow module: controls/count/count.cc
//
// Control to count with a delta on each tick or trigger
//
// <count value="41" delta="1" wait="yes" .../>
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>
#include <stdlib.h>

namespace {

//==========================================================================
// Count control
class CountControl: public Dataflow::Control
{
  double start_value{0.0};
  bool triggered{false};

  // Control virtuals
  void setup() override;
  void pre_tick(const TickData& td) override;
  void notify_target_of(const string& property) override;
  void enable() override;

public:
  double value{0.0};
  double delta{1.0};
  bool wait{false};
  using Control::Control;

  void trigger();
};

//--------------------------------------------------------------------------
// Setup after configuration
void CountControl::setup()
{
  start_value = value;
}

//--------------------------------------------------------------------------
// Enable (reset)
void CountControl::enable()
{
  triggered = false;
  value = start_value;
}

//--------------------------------------------------------------------------
// Automatically set wait flag if we are the trigger target of something
void CountControl::notify_target_of(const string& property)
{
  if (property == "trigger")
    wait = true;
}

//--------------------------------------------------------------------------
// Trigger
void CountControl::trigger()
{
  triggered = true;
}

//--------------------------------------------------------------------------
// Tick
void CountControl::pre_tick(const TickData& /*td*/)
{
  if (wait)
  {
    if (!triggered) return;
    triggered = false;
  }

  value += delta;
  send(Dataflow::Value{value});
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "count",
  "Count",
  "Count a value up or down",
  "core",
  {
    { "value", { "Current value", Value::Type::number,
                 &CountControl::value, true } },
    { "delta", { "Value to change by", Value::Type::number,
                 &CountControl::delta, true } },
    { "wait",  { "Whether to wait for a trigger", Value::Type::boolean,
                 &CountControl::wait, true } },
    { "trigger", { "Trigger to make modification", Value::Type::trigger,
                   &CountControl::trigger, true } }
  },
  { { "", { "Modified value", "", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(CountControl, module)
