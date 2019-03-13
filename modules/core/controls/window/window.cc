//==========================================================================
// ViGraph dataflow module: controls/window/window.cc
//
// Animation control to window a control setting into a fixed range
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Window control
class WindowControl: public Dataflow::Control
{
  string property;
  double min = 0.0;
  double max = 1.0;

  // Control/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  Dataflow::Value::Type get_property_type(const string& property) override;

public:
  // Construct
  WindowControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//   <window min="-0.5" max="0.5" property="x"/>
WindowControl::WindowControl(const Module *module, const XML::Element& config):
  Control(module, config)
{
  property = config["property"];
  min = config.get_attr_real("min", min);
  max = config.get_attr_real("max", max);
}

//--------------------------------------------------------------------------
// Set a control property
void WindowControl::set_property(const string& prop,
                               const SetParams& sp)
{
  if (prop != property) return;

  // Window the value
  if (sp.v.d > max)
    return;
  else if (sp.v.d < min)
    return;

  SetParams nsp(Dataflow::Value{sp.v.d});
  send(nsp);
}

//--------------------------------------------------------------------------
// Get control property types
Dataflow::Value::Type
  WindowControl::get_property_type(const string& prop)
{
  if (prop == property)
    return Dataflow::Value::Type::number;

  return Dataflow::Value::Type::invalid;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "window",
  "Window",
  "Window a value",
  "core",
  {
    { "property", { "Property to set", Value::Type::text } },
    { "min", { "Minimum value", Value::Type::number } },
    { "max", { { "Maximum value", "1.0" }, Value::Type::number } }
  },
  { { "", { "Value output", "", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(WindowControl, module)
