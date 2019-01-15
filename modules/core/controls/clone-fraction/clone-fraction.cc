//==========================================================================
// ViGraph dataflow module: controls/clone-fraction/clone-fraction.cc
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
// CloneFraction control
class CloneFractionControl: public Dataflow::Control
{
  // Control virtuals
  void tick(Dataflow::timestamp_t t) override;

public:
  // Construct
  CloneFractionControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
// <clone-fraction property="..."/>
CloneFractionControl::CloneFractionControl(const Module *module,
                                       const XML::Element& config):
  Element(module, config), Control(module, config)
{
}

//--------------------------------------------------------------------------
// Tick
void CloneFractionControl::tick(Dataflow::timestamp_t /*t*/)
{
  Value v = graph->get_variable("clone-fraction");
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
  "clone-fraction",
  "CloneFraction",
  "Get the fraction (0..1] of a clone",
  "core",
  { },
  { { "", { "Clone fraction", "", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(CloneFractionControl, module)
