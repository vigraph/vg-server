//==========================================================================
// ViGraph dataflow module: vector/filters/translate/translate.cc
//
// Translate filter
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include <cmath>

namespace {

//==========================================================================
// Translate filter
class TranslateFilter: public FrameFilter
{
  Vector start_d, d;

  // Filter/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FramePtr frame) override;
  void enable() override;

public:
  // Construct
  TranslateFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//  <translate x="0.5" y="1.0" z="0.33"/>
TranslateFilter::TranslateFilter(const Dataflow::Module *module,
                                 const XML::Element& config):
  FrameFilter(module, config)
{
  d.x = config.get_attr_real("x");
  d.y = config.get_attr_real("y");
  d.z = config.get_attr_real("z");
  start_d = d;
}

//--------------------------------------------------------------------------
// Set a control property
void TranslateFilter::set_property(const string& property, const SetParams& sp)
{
       if (property == "x") update_prop(d.x, sp);
  else if (property == "y") update_prop(d.y, sp);
  else if (property == "z") update_prop(d.z, sp);
}

//--------------------------------------------------------------------------
// Enable (reset)
void TranslateFilter::enable()
{
  d = start_d;
}

//--------------------------------------------------------------------------
// Process some data
void TranslateFilter::accept(FramePtr frame)
{
  // Modify all points in the frame
  for(auto& p: frame->points) p+=d;
  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "translate",
  "Translate",
  "3D translation",
  "vector",
  {
    { "x", { "Distance to move along X axis", Value::Type::number, "x", true } },
    { "y", { "Distance to move along Y axis", Value::Type::number, "y", true } },
    { "z", { "Distance to move along Z axis", Value::Type::number, "z", true } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(TranslateFilter, module)
