//==========================================================================
// ViGraph dataflow module: vector/filters/sum/sum.cc
//
// Simple filter which vector sums its inputs
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"

namespace {

//==========================================================================
// Sum filter
class SumFilter: public FrameFilter
{
  shared_ptr<Frame> sum{nullptr};

  // Process some data
  void accept(FramePtr frame) override;

  // Notify before and after a tick
  void pre_tick(const TickData&) override;
  void post_tick(const TickData&) override;

public:
  using FrameFilter::FrameFilter;
};

//--------------------------------------------------------------------------
// Process some data
void SumFilter::accept(FramePtr frame)
{
  // If this is the first, just keep it
  if (!sum)
  {
    // Take it over
    sum = frame;
  }
  else
  {
    // Add to existing, up to matching size
    auto n = min(sum->points.size(), frame->points.size());
    for(auto i=0u; i<n; i++)
      sum->points[i] += frame->points[i];
  }
}

//--------------------------------------------------------------------------
// Pre-tick setup
void SumFilter::pre_tick(const TickData&)
{
  sum.reset();
}

//--------------------------------------------------------------------------
// Post-tick flush
void SumFilter::post_tick(const TickData&)
{
  if (!!sum) send(sum);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "sum",
  "Vector sum",
  "Adds multiple input frames, point by point",
  "vector",
  {}, // no properties
  { { "VectorFrame", true } }, // multiple inputs
  { "VectorFrame" }            // one output
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SumFilter, module)
