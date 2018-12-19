//==========================================================================
// ViGraph dataflow module: controls/multiply/multiply.cc
//
// Control to multiply a control value by a factor
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Multiply control
class MultiplyControl: public Dataflow::Control
{
  string property;
  double factor;

  // Mimic of controlled value
  double value{0.0};

  // Control/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  Dataflow::Value::Type get_property_type(const string& property) override;

public:
  // Construct
  MultiplyControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//   <multiply factor="2" property="x"/>
MultiplyControl::MultiplyControl(const Module *module, const XML::Element& config):
  Element(module, config), Control(module, config)
{
  property = config["property"];
  factor = config.get_attr_real("factor", 1.0);
}

//--------------------------------------------------------------------------
// Set a control property
void MultiplyControl::set_property(const string& prop,
                               const SetParams& sp)
{
  if (prop != property) return;

  // Keep our copy up to date
  update_prop(value, sp);

  // Multiply the value
  value *= factor;
  SetParams nsp(Dataflow::Value{value});
  send(nsp);
}

//--------------------------------------------------------------------------
// Get control property types
Dataflow::Value::Type
  MultiplyControl::get_property_type(const string& prop)
{
  if (prop == property)
    return Dataflow::Value::Type::number;

  return Dataflow::Value::Type::invalid;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "multiply",
  "Multiply",
  "Multiply a control value",
  "core",
  {
    { "property", { "Property to set", Value::Type::text } },
    { "factor", { { "Factor to multiply with", "1.0" }, Value::Type::number } }
  },
  { { "", { "Value output", "", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MultiplyControl, module)
