//==========================================================================
// ViGraph dataflow module: controls/multiply/multiply.cc
//
// Control to multiply a control value by a factor
//
//  <multiply factor="2" .../>
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
  // Control/Element virtuals
  void update();

public:
  double value{0.0};
  double factor{1.0};
  using Control::Control;
};

//--------------------------------------------------------------------------
// Update after value set
void MultiplyControl::update()
{
  // Multiply by factor and pass on
  send(Dataflow::Value{value*factor});
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
    { "value", { "Base value", Value::Type::number,
                  &MultiplyControl::value, true } },
    { "factor", { "Factor to multiply with", Value::Type::number,
                  &MultiplyControl::factor, true } }
  },
  { { "value", { "Value output", "value", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MultiplyControl, module)
