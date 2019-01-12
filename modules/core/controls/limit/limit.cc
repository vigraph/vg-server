//==========================================================================
// ViGraph dataflow module: controls/limit/limit.cc
//
// Animation control to limit a control setting into a fixed range
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Limit control
class LimitControl: public Dataflow::Control
{
  string property;
  double min, max;
  double initial_value;

  // Mimic of controlled value
  double value{0.0};

  // Control/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  Dataflow::Value::Type get_property_type(const string& property) override;
  void enable() override;

public:
  // Construct
  LimitControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//   <limit value="0.0" min="-0.5" max="0.5" property="x"/>
LimitControl::LimitControl(const Module *module, const XML::Element& config):
  Element(module, config), Control(module, config)
{
  property = config["property"];
  min = config.get_attr_real("min", 0.0);
  max = config.get_attr_real("max", 1.0);
  value = initial_value = config.get_attr_real("value");
}

//--------------------------------------------------------------------------
// Enable (reset)
void LimitControl::enable()
{
  value = initial_value;
}

//--------------------------------------------------------------------------
// Set a control property
void LimitControl::set_property(const string& prop,
                               const SetParams& sp)
{
  if (prop != property) return;

  // Keep our copy up to date
  update_prop(value, sp);

  // Limit the value
  if (value > max)
    value = max;
  else if (value < min)
    value = min;

  SetParams nsp(Dataflow::Value{value});
  send(nsp);
}

//--------------------------------------------------------------------------
// Get control property types
Dataflow::Value::Type
  LimitControl::get_property_type(const string& prop)
{
  if (prop == property)
    return Dataflow::Value::Type::number;

  return Dataflow::Value::Type::invalid;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "limit",
  "Limit",
  "Hard limit on a value",
  "core",
  {
    { "property", { "Property to set", Value::Type::text } },
    { "min", { "Minimum value", Value::Type::number } },
    { "max", { { "Maximum value", "1.0" }, Value::Type::number } }
  },
  { { "", { "Value output", "", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(LimitControl, module)
