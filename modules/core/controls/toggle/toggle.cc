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
private:
  bool multi = false;
  bool state = false;
  map<double, bool> multistate;

  // Control/Element virtuals
  void notify_target_of(Element *, const string& property) override;

public:
  double value = 0.0;

  // Construct
  using Control::Control;

  void toggle();
};

//--------------------------------------------------------------------------
// Trigger toggle
void ToggleControl::toggle()
{
  if (multi)
  {
    auto& s = multistate[value];
    s = !s;
    send(s ? "trigger" : "clear", {});
  }
  else
  {
    state = !state;
    send(state ? "trigger" : "clear", {});
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
    { "value", { "Set value to toggle on", Value::Type::number,
                 &ToggleControl::value, true }},
    { "trigger", { "Trigger toggle", Value::Type::trigger,
                   &ToggleControl::toggle, true }},
  },
  {
    { "trigger", { "Trigger on", "trigger", Value::Type::trigger }},
    { "clear", { "Clear", "clear", Value::Type::trigger }},
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ToggleControl, module)
