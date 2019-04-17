//==========================================================================
// ViGraph dataflow module: controls/wrap/wrap.cc
//
// Animation control to wrap a control setting into a fixed range
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Wrap control
class WrapControl: public Dataflow::Control
{
private:
  void update() override;

public:
  double min = 0.0;
  double max = 1.0;
  double value{0.0};
  using Control::Control;
};

//--------------------------------------------------------------------------
// Set a control property
void WrapControl::update()
{
  // Wrap the value
  if (value >= max)
    value = min + fmod(value-min, max-min);
  else if (value < min)
    value = max - fmod(max-value, max-min);

  send(Dataflow::Value{value});
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "wrap",
  "Wrap",
  "Wrap a value into a range",
  "core",
  {
    { "value", { "Base value", Value::Type::number,
                 &WrapControl::value, true } },
    { "min", { "Minimum value", Value::Type::number,
               &WrapControl::min, true} },
    { "max", { "Maximum value", Value::Type::number,
               &WrapControl::max, true } },
  },
  { { "value", { "Value output", "value", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(WrapControl, module)
