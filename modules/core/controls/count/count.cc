//==========================================================================
// ViGraph dataflow module: controls/count/count.cc
//
// Control to count with a delta on each tick or trigger
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
  // Configured state
  double start_value{0.0};
  double delta{0.0};
  bool wait{false};

  // Dynamic state
  double value{0.0};
  bool triggered{false};

  // Control virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void pre_tick(const TickData& td) override;
  void notify_target_of(Element *, const string& property) override;
  void enable() override;

public:
  // Construct
  CountControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
// <count value="41" delta="1" wait="yes" .../>
CountControl::CountControl(const Module *module, const XML::Element& config):
  Control(module, config)
{
  value = start_value = config.get_attr_real("value", 0.0);
  delta = config.get_attr_real("delta", 1.0);
  wait = config.get_attr_bool("wait");
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
void CountControl::notify_target_of(Element *, const string& property)
{
  if (property == "trigger")
    wait = true;
}

//--------------------------------------------------------------------------
// Set a control property
void CountControl::set_property(const string& property, const SetParams& sp)
{
  if (property == "trigger")
    triggered = true;
  else if (property == "value")
    update_prop(value, sp);
  else if (property == "delta")
    update_prop(delta, sp);
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
    { "value", { "Current value", Value::Type::number, true } },
    { "delta", { "Value to change by", Value::Type::number, true } },
    { "wait",  { "Whether to wait for a trigger", Value::Type::number } },
    { "trigger", { "Trigger to make modification", Value::Type::trigger, true}}
  },
  { { "", { "Modified value", "", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(CountControl, module)
