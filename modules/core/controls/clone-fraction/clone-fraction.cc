//==========================================================================
// ViGraph dataflow module: controls/clone-fraction/clone-fraction.cc
//
// Control to randomise properties on other elements
//
// <clone-fraction scale="1.0" offset="0" .../>
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>
#include <stdlib.h>

namespace {

//==========================================================================
// CloneFraction control
class CloneFractionControl: public Dataflow::Control
{
public:
  double scale{1.0};
  double offset{0.0};

private:
  // Control virtuals
  void pre_tick(const TickData& td) override;

public:
  // Construct
  using Control::Control;
};

//--------------------------------------------------------------------------
// Tick
void CloneFractionControl::pre_tick(const TickData& /*td*/)
{
  Value v = graph->get_variable("clone-fraction");
  if (v.type != Value::Type::invalid)
  {
    v.d *= scale;
    v.d += offset;
    send(v);
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "clone-fraction",
  "CloneFraction",
  "Get the fraction (0..1] of a clone",
  "core",
  {
    { "scale",  { "Scale to apply to fraction", Value::Type::number,
                  &CloneFractionControl::scale, true } },
    { "offset", { "Offset to apply to fraction", Value::Type::number,
                  &CloneFractionControl::offset, true } }
  },
  { { "value", { "Clone fraction", "value", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(CloneFractionControl, module)
