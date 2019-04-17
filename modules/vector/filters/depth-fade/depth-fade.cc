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
public:
  double distance = 0;  // Black distance

private:
  // Filter/Element virtuals
  void accept(FramePtr frame) override;

public:
  using FrameFilter::FrameFilter;
};

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
    { "d", { "Distance to fade over", Value::Type::number,
             &DepthFadeFilter::distance, true } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(DepthFadeFilter, module)
