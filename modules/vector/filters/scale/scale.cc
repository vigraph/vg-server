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
public:
  double sx{0.0};
  double sy{0.0};
  double sz{0.0};

private:
  // Filter/Element virtuals
  void accept(FramePtr frame) override;

public:
  using FrameFilter::FrameFilter;

  // Getters/Setters
  void set_all(double s) { sx = s; sy = s; sz = s; }
};

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
    { "all", { "Factor to scale all axes", Value::Type::number,
        { &ScaleFilter::set_all }, true, true } },
    { "x", { "Factor to scale X", Value::Type::number,
             &ScaleFilter::sx, true } },
    { "y", { "Factor to scale Y", Value::Type::number,
             &ScaleFilter::sy, true } },
    { "z", { "Factor to scale Z", Value::Type::number,
             &ScaleFilter::sz, true } },
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ScaleFilter, module)
