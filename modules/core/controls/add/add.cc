//==========================================================================
// ViGraph dataflow module: controls/add/add.cc
//
// Animation control to add a value to a control setting
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Add control
class AddControl: public Dataflow::Control
{
  string property;
  double offset;

  // Mimic of controlled value
  double value{0.0};

  // Control/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  Dataflow::Value::Type get_property_type(const string& property) override;

public:
  // Construct
  AddControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//   <add offset="0.1" property="x"/>
AddControl::AddControl(const Module *module, const XML::Element& config):
  Element(module, config), Control(module, config)
{
  property = config["property"];
  offset = config.get_attr_real("offset");
}

//--------------------------------------------------------------------------
// Set a control property
void AddControl::set_property(const string& prop,
                              const SetParams& sp)
{
  if (prop != property) return;

  // Keep our copy up to date
  update_prop(value, sp);

  // Add the offset and pass on
  value += offset;
  SetParams nsp(Dataflow::Value{value});
  send(nsp);
}

//--------------------------------------------------------------------------
// Get control property types
Dataflow::Value::Type
  AddControl::get_property_type(const string& prop)
{
  if (prop == property)
    return Dataflow::Value::Type::number;

  return Dataflow::Value::Type::invalid;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "add",
  "Add",
  "Add an offset to a control value",
  "core",
  {
    { "property", { "Property to set", Value::Type::text } },
    { "offset", { "Offset amount", Value::Type::number } },
  },
  { { "", { "Value output", "", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AddControl, module)
