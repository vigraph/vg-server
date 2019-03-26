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
  void update();

public:
  double value{0.0};
  double offset;
  using Control::Control;
};

//--------------------------------------------------------------------------
// Update after value set
void AddControl::update()
{
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
          static_cast<double Element::*>(&AddControl::value), true } },
    { "offset",
      { "Offset amount", Value::Type::number,
          static_cast<double Element::*>(&AddControl::offset), true } },
  },
  { { "", { "Value output", "", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AddControl, module)
