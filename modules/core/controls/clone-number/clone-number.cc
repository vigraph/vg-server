//==========================================================================
// ViGraph dataflow module: controls/clone-number/clone-number.cc
//
// Control to randomise properties on other elements
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>
#include <stdlib.h>

namespace {

//==========================================================================
// CloneNumber control
class CloneNumberControl: public Dataflow::Control
{
  // Control virtuals
  void pre_tick(const TickData& td) override;

public:
  using Control::Control;
};

//--------------------------------------------------------------------------
// Tick
void CloneNumberControl::pre_tick(const TickData& /*td*/)
{
  Value v = graph->get_variable("clone-number");
  if (v.type != Value::Type::invalid)
    send(v);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "clone-number",
  "CloneNumber",
  "Get the number (1..N) of a clone",
  "core",
  { },
  { { "output", { "Clone number", "value", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(CloneNumberControl, module)
