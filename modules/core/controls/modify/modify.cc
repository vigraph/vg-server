//==========================================================================
// ViGraph dataflow module: controls/modify/modify.cc
//
// Control to modify properties on other elements with a delta
// (or flip for boolean)
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>
#include <stdlib.h>

namespace {

//==========================================================================
// Modify control
class ModifyControl: public Dataflow::Control
{
  // Configured state
  double delta{0.0};
  bool wait{false};

  // Dynamic state
  bool done{false};
  bool triggered{false};

  // Control virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void tick(const TickData& td) override;
  void notify_target_of(Element *, const string& property) override;

public:
  // Construct
  ModifyControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
// <modify delta="42" type="{real|integer}" wait="yes"
//      property="..."/>
ModifyControl::ModifyControl(const Module *module, const XML::Element& config):
  Element(module, config), Control(module, config)
{
  delta = config.get_attr_real("delta", 1.0);
  wait = config.get_attr_bool("wait");
}

//--------------------------------------------------------------------------
// Automatically set wait flag if we are the trigger target of something
void ModifyControl::notify_target_of(Element *, const string& property)
{
  if (property == "trigger")
    wait = true;
}

//--------------------------------------------------------------------------
// Set a control property
void ModifyControl::set_property(const string& property, const SetParams& sp)
{
  if (property == "trigger")
    triggered = true;
  else if (property == "delta")
    update_prop(delta, sp);
}

//--------------------------------------------------------------------------
// Tick
void ModifyControl::tick(const TickData& /*td*/)
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

  SetParams sp(Dataflow::Value{delta}, true); // increment
  send(sp);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "modify",
  "Modify",
  "Modify (increment/decrement) a value on another element",
  "core",
  {
    { "delta", { "Value to change by", Value::Type::number, true } },
    { "wait",  { "Whether to wait for a trigger", Value::Type::number } },
    { "trigger", { "Trigger to make modification", Value::Type::trigger, true}}
  },
  { { "", { "Modified value", "", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ModifyControl, module)
