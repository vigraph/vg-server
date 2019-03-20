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
  // Control/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;

public:
  double value{0.0};
  double offset;

  // Construct
  AddControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//   <add offset="0.1"/>
AddControl::AddControl(const Module *module, const XML::Element& config):
  Control(module, config)
{
  offset = config.get_attr_real("offset");
}

//--------------------------------------------------------------------------
// Set a control property
void AddControl::set_property(const string& prop,
                              const SetParams& sp)
{
  if (prop == "value")
    update_prop(value, sp);
  else if (prop == "offset")
    update_prop(offset, sp);

  // Add the offset and pass on
  SetParams nsp(Dataflow::Value{value+offset});
  send(nsp);
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
    { "value",
      { "Base value", Value::Type::number,
          static_cast<double Element::*>(&AddControl::value) } },
    { "offset",
      { "Offset amount", Value::Type::number,
          static_cast<double Element::*>(&AddControl::offset) } },
  },
  { { "", { "Value output", "", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AddControl, module)
