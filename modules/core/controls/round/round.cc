//==========================================================================
// ViGraph dataflow module: controls/round/round.cc
//
// Control to round a value of a control setting
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"

namespace {

//==========================================================================
// Round control
class RoundControl: public Dataflow::Control
{
  string property;
  double n = 1.0; // Enumerator
  double d = 1.0; // Divisor

  // Control/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  Dataflow::Value::Type get_property_type(const string& property) override;

public:
  // Construct
  RoundControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//   <round property="x" n="1" d="2" />
RoundControl::RoundControl(const Module *module, const XML::Element& config):
  Element(module, config), Control(module, config)
{
  property = config["property"];
  n = config.get_attr_int("n", n);
  d = config.get_attr_int("d", d);
}

//--------------------------------------------------------------------------
// Set a control property
void RoundControl::set_property(const string& prop, const SetParams& sp)
{
  if (prop == property)
  {
    if (!n || !d)
      return;
    // Round the value and pass on
    const auto value = n * round(d * sp.v.d / n) / d;
    SetParams nsp(Dataflow::Value{value});
    send(nsp);
  }
  else if (prop == "n")
  {
    update_prop(n, sp);
  }
  else if (prop == "d")
  {
    update_prop(d, sp);
  }
}

//--------------------------------------------------------------------------
// Get control property types
Dataflow::Value::Type RoundControl::get_property_type(const string& prop)
{
  if (prop == property)
    return Dataflow::Value::Type::number;
  else if (prop == "n")
    return Dataflow::Value::Type::number;
  else if (prop == "d")
    return Dataflow::Value::Type::number;
  else
    return Dataflow::Value::Type::invalid;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "round",
  "Round",
  "Round a control value",
  "core",
  {
    { "property", { "Property to set", Value::Type::text } },
    { "n", { {"Enumerator", ""}, Value::Type::number, "@n", true } },
    { "d", { {"Divisor", ""}, Value::Type::number, "@d", true } },
  },
  { { "", { "Value output", "", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(RoundControl, module)
