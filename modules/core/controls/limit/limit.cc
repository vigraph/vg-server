//==========================================================================
// ViGraph dataflow module: controls/limit/limit.cc
//
// Control to limit a control setting into a fixed range
//
//   <limit value="0.0" min="-0.5" max="0.5" .../>
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Limit control
class LimitControl: public Dataflow::Control
{
  void update() override;

public:
  double min{0.0};
  double max{1.0};
  double value{0.0};
  using Control::Control;
};

//--------------------------------------------------------------------------
// Update after property set
void LimitControl::update()
{
  // Limit the value
  if (value > max)
    value = max;
  else if (value < min)
    value = min;

  send(Dataflow::Value{value});
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "limit",
  "Limit",
  "Hard limit on a value",
  "core",
  {
    { "value", { "Base value", Value::Type::number,
                 &LimitControl::value, true } },
    { "min", { "Minimum value", Value::Type::number,
               &LimitControl::min, true } },
    { "max", { "Maximum value", Value::Type::number,
               &LimitControl::max, true } }
  },
  { { "output", { "Value output", "value", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(LimitControl, module)
