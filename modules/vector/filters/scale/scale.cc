//==========================================================================
// ViGraph dataflow module: vector/filters/scale/scale.cc
//
// Scale filter
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include <cmath>

namespace {

//==========================================================================
// Scale filter
class ScaleFilter: public FrameFilter
{
  double sx{0.0};
  double sy{0.0};
  double sz{0.0};

  // Filter/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FramePtr frame) override;

public:
  // Construct
  ScaleFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//  <scale all="0.5"/>
//  <scale x="0.5" y="1.0" z="2.0"/>
ScaleFilter::ScaleFilter(const Dataflow::Module *module,
                         const XML::Element& config):
  FrameFilter(module, config)
{
  if (config.has_attr("all"))
    sx = sy = sz = config.get_attr_real("all", 1.0);
  else
  {
    sx = config.get_attr_real("x", 1.0);
    sy = config.get_attr_real("y", 1.0);
    sz = config.get_attr_real("z", 1.0);
  }
}

//--------------------------------------------------------------------------
// Set a control property
void ScaleFilter::set_property(const string& property, const SetParams& sp)
{
  if (property == "all")
  { update_prop(sx, sp); update_prop(sy, sp); update_prop(sz, sp); }
  else if (property == "x") update_prop(sx, sp);
  else if (property == "y") update_prop(sy, sp);
  else if (property == "z") update_prop(sz, sp);

}

//--------------------------------------------------------------------------
// Process some data
void ScaleFilter::accept(FramePtr frame)
{
  // Modify all points in the frame
  for(auto& p: frame->points)
  {
    p.x *= sx;
    p.y *= sy;
    p.z *= sz;
  }

  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "scale",
  "Scale",
  "3D scaling",
  "vector",
  {
    { "all", { {"Factor to scale all axes","1.0"}, Value::Type::number,
                                                     "all", true } },
    { "x", { {"Factor to scale X","1.0"}, Value::Type::number, "x", true } },
    { "y", { {"Factor to scale Y","1.0"}, Value::Type::number, "y", true } },
    { "z", { {"Factor to scale Z","1.0"}, Value::Type::number, "z", true } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ScaleFilter, module)
