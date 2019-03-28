//==========================================================================
// ViGraph dataflow module: filters/fade.cc
//
// Global brightness fade
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include <cmath>

namespace {

//==========================================================================
// Fade filter
class FadeFilter: public FrameFilter
{
public:
  double fr{1.0};
  double fg{1.0};
  double fb{1.0};

private:
  // Filter/Element virtuals
  void accept(FramePtr frame) override;

public:
  using FrameFilter::FrameFilter;

  // Getters/Setters
  void set_all(double f) { fr = f; fg = f; fb = f; }
};

//--------------------------------------------------------------------------
// Process some data
void FadeFilter::accept(FramePtr frame)
{
  // If fading to black, kill the frame altogether
  if (!fr && !fg && !fb)
  {
    frame->points.clear();
    send(frame);
    return;
  }

  // Modify all points in the frame
  for(auto& p: frame->points)
  {
    p.c.r *= fr;
    p.c.g *= fg;
    p.c.b *= fb;
  }

  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "fade",
  "Fade",
  "Fade all points to a multiple of their brightness",
  "vector",
  {
    { "all", { "Factor to fade all channels", Value::Type::number,
               { &FadeFilter::set_all },
               true, true } },
    { "r", { "Factor to fade red", Value::Type::number,
             &FadeFilter::fr, true } },
    { "g", { "Factor to fade green", Value::Type::number,
             &FadeFilter::fg, true } },
    { "b", { "Factor to fade blue", Value::Type::number,
             &FadeFilter::fb, true } },
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(FadeFilter, module)
