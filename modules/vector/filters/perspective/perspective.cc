//==========================================================================
// ViGraph dataflow module: vector/filters/perspective/perspective.cc
//
// 3D perspective filter
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include <cmath>

namespace {

//==========================================================================
// Perspective filter
class PerspectiveFilter: public FrameFilter
{
public:
  double distance = 1;  // Horizon distance

private:
  // Filter/Element virtuals
  void accept(FramePtr frame) override;

public:
  using FrameFilter::FrameFilter;
};

//--------------------------------------------------------------------------
// Process some data
void PerspectiveFilter::accept(FramePtr frame)
{
  // Modify all points in the frame
  for(auto& p: frame->points)
  {
    if (distance+p.z > 0)
    {
      double f = distance / (distance+p.z);
      p.x *= f;
      p.y *= f;
    }
    else p.c = Colour::black;  // blank it
  }

  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "perspective",
  "Perspective",
  "Perspective scaling over Z distance",
  "vector",
  {
    { "d", { "Horizon distance", Value::Type::number,
             &PerspectiveFilter::distance, true } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(PerspectiveFilter, module)
