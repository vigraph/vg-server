//==========================================================================
// ViGraph dataflow module: controls/scale/scale.cc
//
// Control to provide a incremental scale based on a factor
//
//  <scale factor="2" .../>
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Scale control
class ScaleControl: public Dataflow::Control
{
  // Control/Element virtuals
  void update();

public:
  double value{1.0};
  double factor{1.0};
  using Control::Control;

  void inc() { if (factor) value *= factor; }
  void dec() { if (factor) value /= factor; }
};

//--------------------------------------------------------------------------
// Update after value set
void ScaleControl::update()
{
  send(Dataflow::Value{value});
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "scale",
  "Scale",
  "Scale a control value",
  "core",
  {
    { "value", { "Value", Value::Type::number,
                  &ScaleControl::value, true } },
    { "factor", { "Factor to increment by", Value::Type::number,
                  &ScaleControl::factor, true } },
    { "inc", { "Increment value by scaling factor", Value::Type::trigger,
               &ScaleControl::inc, true } },
    { "dec", { "Decrement value by scaling factor", Value::Type::trigger,
               &ScaleControl::dec, true } },
  },
  { { "output", { "Value output", "value", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ScaleControl, module)
