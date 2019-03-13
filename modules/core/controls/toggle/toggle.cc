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
  bool state = false;
  map<Value, bool> multistate;

  // Control/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;

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
  if (prop == "trigger")
  {
    state = !state;
    send(state ? "trigger" : "clear", {});
  }
  else if (prop == "on")
  {
    auto& s = multistate[sp.v];
    s = !s;
    send(s ? "on" : "off", sp);
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "toggle",
  "Toggle",
  "Toggle an a control on/off",
  "core",
  {
    { "trigger", { "Note trigger", Value::Type::trigger, "@trigger", true }},
    { "on", { "Note on", Value::Type::number, "@on", true }},
    { "off", { "Note off", Value::Type::number, "@off", true }},
  },
  {
    { "trigger", { "Note trigger", "trigger", Value::Type::trigger }},
    { "clear", { "Note clear", "clear", Value::Type::trigger }},
    { "on", { "Note on", "on", Value::Type::number }},
    { "off", { "Note off", "off", Value::Type::number }},
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ToggleControl, module)
