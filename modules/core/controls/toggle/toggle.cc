//==========================================================================
// ViGraph dataflow module: controls/toggle/toggle.cc
//
// Control to toggle a control on/off
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Toggle control
class ToggleControl: public Dataflow::Control
{
  bool multi = false;
  bool state = false;
  Value v;
  map<Value, bool> multistate;

  // Control/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void notify_target_of(Element *, const string& property) override;

public:
  // Construct
  ToggleControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//   <toggle property="x"/>
ToggleControl::ToggleControl(const Module *module, const XML::Element& config):
  Control(module, config)
{
}

//--------------------------------------------------------------------------
// Set a control property
void ToggleControl::set_property(const string& prop,
                                 const SetParams& sp)
{
  if (prop == "value")
  {
    v = sp.v;
  }
  else if (prop == "trigger")
  {
    if (multi)
    {
      auto& s = multistate[v];
      s = !s;
      send(s ? "trigger" : "clear", {});
    }
    else
    {
      state = !state;
      send(state ? "trigger" : "clear", {});
    }
  }
}

//--------------------------------------------------------------------------
// Set to multi-value toggle if target of value
void ToggleControl::notify_target_of(Element *, const string& property)
{
  if (property == "value")
    multi = true;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "toggle",
  "Toggle",
  "Toggle a control on/off",
  "core",
  {
    { "value", { "Set value to toggle on", Value::Type::number, true }},
    { "trigger", { "Trigger toggle", Value::Type::trigger, true }},
  },
  {
    { "trigger", { "Trigger on", "trigger", Value::Type::trigger }},
    { "clear", { "Clear", "clear", Value::Type::trigger }},
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ToggleControl, module)
