//==========================================================================
// ViGraph dataflow module: controls/add/add.cc
//
// Animation control to add a value to a control setting
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"

namespace {

//==========================================================================
// Add control
class AddControl: public Dataflow::Control
{
  // Control/Element virtuals
  void update();

public:
  double value{0.0};
  double offset{0.0};
  using Control::Control;
};

//--------------------------------------------------------------------------
// Update after value set
void AddControl::update()
{
  // Add the offset and pass on
  send(Dataflow::Value{value+offset});
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
          static_cast<double Element::*>(&AddControl::value), true } },
    { "offset",
      { "Offset amount", Value::Type::number,
          static_cast<double Element::*>(&AddControl::offset), true } },
  },
  { { "value", { "Value output", "value", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AddControl, module)
