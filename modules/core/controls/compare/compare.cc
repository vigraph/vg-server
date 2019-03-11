//==========================================================================
// ViGraph dataflow module: controls/compare/compare.cc
//
// Animation control to compare a control setting into a fixed range
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Compare control
class CompareControl: public Dataflow::Control
{
  string property;
  double min = 0.0;
  double max = 1.0;
  bool on_change = false;
  bool last_result = false;

  // Control/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  Dataflow::Value::Type get_property_type(const string& property) override;

public:
  // Construct
  CompareControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//   <compare min="-0.5" max="0.5" property="x"/>
CompareControl::CompareControl(const Module *module, const XML::Element& config):
  Element(module, config), Control(module, config)
{
  property = config["property"];
  min = config.get_attr_real("min", min);
  max = config.get_attr_real("max", max);
  on_change = config.get_attr_bool("on-change", on_change);
}

//--------------------------------------------------------------------------
// Set a control property
void CompareControl::set_property(const string& prop,
                               const SetParams& sp)
{
  if (prop != property) return;

  if (sp.v.d > max || sp.v.d < min)
  {
    if (!on_change || last_result)
    {
      send("clear", Dataflow::Value{});
      last_result = false;
    }
  }
  else
  {
    if (!on_change || !last_result)
    {
      send("trigger", Dataflow::Value{});
      last_result = true;
    }
  }
}

//--------------------------------------------------------------------------
// Get control property types
Dataflow::Value::Type
  CompareControl::get_property_type(const string& prop)
{
  if (prop == property)
    return Dataflow::Value::Type::number;

  return Dataflow::Value::Type::invalid;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "compare",
  "Compare",
  "Compare a value",
  "core",
  {
    { "property", { "Property to compare", Value::Type::text } },
    { "min", { "Minimum value", Value::Type::number } },
    { "max", { { "Maximum value", "1.0" }, Value::Type::number } },
    { "on-change", { { "Send triggers only on change", "false" },
                     Value::Type::boolean } },
  },
  {
    { "trigger", { "Match", "trigger", Value::Type::trigger }},
    { "clear", { "No match", "clear", Value::Type::trigger }},
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(CompareControl, module)
