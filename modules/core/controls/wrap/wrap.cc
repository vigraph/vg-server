//==========================================================================
// ViGraph dataflow module: controls/wrap/wrap.cc
//
// Animation control to wrap a control setting into a fixed range
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Wrap control
class WrapControl: public Dataflow::Control
{
  string property;
  double min, max;

  // Mimic of controlled value
  double value{0.0};

  // Control/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  Dataflow::Value::Type get_property_type(const string& property) override;

public:
  // Construct
  WrapControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//   <wrap value="0.0" min="0.0" max="1.0" property="x"/>
WrapControl::WrapControl(const Module *module, const XML::Element& config):
  Element(module, config), Control(module, config)
{
  property = config["property"];
  min = config.get_attr_real("min", 0.0);
  max = config.get_attr_real("max", 1.0);
  value = config.get_attr_real("value");
}

//--------------------------------------------------------------------------
// Set a control property
void WrapControl::set_property(const string& prop,
                               const SetParams& sp)
{
  if (prop != property) return;

  // Keep our copy up to date
  update_prop(value, sp);

  // Wrap the value
  if (value >= max)
    value = min + fmod(value-min, max-min);
  else if (value < min)
    value = max - fmod(max-value, max-min);

  SetParams nsp(Dataflow::Value{value});
  send(nsp);
}

//--------------------------------------------------------------------------
// Get control property types
Dataflow::Value::Type
  WrapControl::get_property_type(const string& prop)
{
  if (prop == property)
    return Dataflow::Value::Type::number;

  return Dataflow::Value::Type::invalid;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "wrap",
  "Wrap",
  "Wrap a value into a range",
  "core",
  {
    { "property", { "Property to set", Value::Type::text } },
    { "min", { "Minimum value", Value::Type::number} },
    { "max", { { "Maximum value", "1.0" }, Value::Type::number } },
    { "value", { "Initial value", Value::Type::number } }
  },
  { { "", { "Value output", "", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(WrapControl, module)
