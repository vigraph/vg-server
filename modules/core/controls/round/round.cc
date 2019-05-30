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
private:
  // Control/Element virtuals
  void update() override;

public:
  double n = 1.0; // Enumerator
  double d = 1.0; // Divisor
  double value = 0.0;

  // Construct
  using Control::Control;
};

//--------------------------------------------------------------------------
// Update
void RoundControl::update()
{
  if (!n || !d)
    return;
  // Round the value and pass on
  const auto v = n * round(d * value / n) / d;
  send(Dataflow::Value{v});
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
    { "value", { "Value to round", Value::Type::number,
                 &RoundControl::value, true } },
    { "n", { "Enumerator", Value::Type::number, &RoundControl::n , true } },
    { "d", { "Divisor", Value::Type::number, &RoundControl::d, true } },
  },
  { { "output", { "Value output", "value", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(RoundControl, module)
