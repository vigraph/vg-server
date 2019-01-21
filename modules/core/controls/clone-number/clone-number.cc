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
  void tick(const TickData& td) override;

public:
  // Construct
  CloneNumberControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
// <clone-number property="..."/>
CloneNumberControl::CloneNumberControl(const Module *module,
                                       const XML::Element& config):
  Element(module, config), Control(module, config)
{
}

//--------------------------------------------------------------------------
// Tick
void CloneNumberControl::tick(const TickData& /*td*/)
{
  Value v = graph->get_variable("clone-number");
  if (v.type != Value::Type::invalid)
  {
    SetParams sp(v);
    send(sp);
  }
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
  { { "", { "Clone number", "", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(CloneNumberControl, module)
