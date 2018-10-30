//==========================================================================
// ViGraph dataflow module: filters/depth-fade.cc
//
// Fade further away points
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include <cmath>

namespace {

//==========================================================================
// DepthFade filter
class DepthFadeFilter: public FrameFilter
{
  double distance;  // Black distance

  // Filter/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FramePtr frame) override;

public:
  // Construct
  DepthFadeFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//  <depth-fade distance="10"/>
DepthFadeFilter::DepthFadeFilter(const Dataflow::Module *module,
                                 const XML::Element& config):
  Element(module, config), FrameFilter(module, config)
{
  distance = config.get_attr_real("d", 1);
}

//--------------------------------------------------------------------------
// Set a control property
void DepthFadeFilter::set_property(const string& property, const SetParams& sp)
{
  if (property == "d") update_prop(distance, sp);
}

//--------------------------------------------------------------------------
// Process some data
void DepthFadeFilter::accept(FramePtr frame)
{
  // Modify all points in the frame
  for(auto& p: frame->points)
  {
    if (p.z < distance)
    {
      if (p.z > 0)
      {
        double f = distance/(distance-p.z);
        p.c.fade(f);
      }
    }
    else p.c = Colour::black;  // blank it
  }

  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "depth-fade",
  "Depth fade",
  "Depth-cue from fading over Z distance",
  "vector",
  {
    { "d", { {"Distance to fade over","1"}, Value::Type::number, "d", true } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(DepthFadeFilter, module)
