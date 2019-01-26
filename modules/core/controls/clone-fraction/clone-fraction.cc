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
  double scale{1.0};
  double offset{0.0};

  // Control virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void tick(const TickData& td) override;

public:
  // Construct
  CloneFractionControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
// <clone-fraction scale="1.0" offset="0" property="..."/>
CloneFractionControl::CloneFractionControl(const Module *module,
                                       const XML::Element& config):
  Element(module, config), Control(module, config)
{
  scale = config.get_attr_real("scale", 1.0);
  offset = config.get_attr_real("offset");
}

//--------------------------------------------------------------------------
// Set a control property
void CloneFractionControl::set_property(const string& property,
                                        const SetParams& sp)
{
  if (property == "scale")
    update_prop(scale, sp);
  else if (property == "offset")
    update_prop(offset, sp);
}

//--------------------------------------------------------------------------
// Tick
void CloneFractionControl::tick(const TickData& /*td*/)
{
  Value v = graph->get_variable("clone-fraction");
  if (v.type != Value::Type::invalid)
  {
    v.d *= scale;
    v.d += offset;
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
  {
    { "scale",  { {"Scale to apply to fraction", "1.0"},
          Value::Type::number, "@scale", true } },
    { "offset", { {"Offset to apply to fraction", "0"},
          Value::Type::number, "@offset", true } }
  },
  { { "", { "Clone fraction", "", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(CloneFractionControl, module)
