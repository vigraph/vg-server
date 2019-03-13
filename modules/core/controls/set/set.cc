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
  double value{0.0};
  Value::Type type{Value::Type::number};
  bool wait{false};
  Time::Duration delay{-1.0};
  Dataflow::timestamp_t delayed_until = -1.0;
  bool do_delay = false;

  // Dynamic state
  bool done{false};
  bool triggered{false};

  // Control virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void pre_tick(const TickData& td) override;
  void notify_target_of(Element *, const string& property) override;
  void enable() override;

public:
  // Construct
  SetControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
// <set value="42" wait="yes"
//      type="{number|trigger}"
//      property="..."/>
SetControl::SetControl(const Module *module, const XML::Element& config):
  Element(module, config), Control(module, config)
{
  value = config.get_attr_real("value");
  wait = config.get_attr_bool("wait");
  if (config.has_attr("delay"))
    delay = Time::Duration{config["delay"]};

  const string& type_s = config["type"];
  if (type_s.empty() || type_s=="number")
    type = Value::Type::number;
  else if (type_s=="trigger")
    type = Value::Type::trigger;
  else
    throw runtime_error("Unknown value type "+type_s+" in <set> "+id);
}

//--------------------------------------------------------------------------
// Automatically set wait flag if we are the trigger target of something
void SetControl::notify_target_of(Element *, const string& property)
{
  if (property == "trigger")
    wait = true;
}

//--------------------------------------------------------------------------
// Set a control property
void SetControl::set_property(const string& property, const SetParams& sp)
{
  if (property == "trigger")
  {
    triggered = true;
    if (delay.seconds() >= 0)
      do_delay = true;
  }
  else if (property == "value")
  {
    update_prop(value, sp);
    if (delay.seconds() >= 0)
      do_delay = true;
  }
}

//--------------------------------------------------------------------------
// Enable (reset)
void SetControl::enable()
{
  triggered = done = do_delay = false;
  delayed_until = -1.0;
}

//--------------------------------------------------------------------------
// Tick
void SetControl::pre_tick(const TickData& td)
{
  // Apply delay if necessary
  if (do_delay)
  {
    delayed_until = td.t + delay.seconds();
    do_delay = false;
    return;
  }

  if (delayed_until >= 0 && delayed_until > td.t)
    return;
  delayed_until = -1.0;

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
  Value v;

  switch (type)
  {
    default:
    case Value::Type::number:
      v = Value{value};
      break;

    case Value::Type::trigger:
      // trigger is the default Value
      break;
  }

  SetParams sp(v);
  send(sp);
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
    { "value", { "Value to set", Value::Type::number, true } },
    { "wait",  { "Whether to wait for a trigger", Value::Type::number } },
    { "trigger", { "Trigger to set value", Value::Type::trigger, true } }
  },
  { { "", { "Any value", "", Value::Type::any }}} // Flexible controlled property
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SetControl, module)
