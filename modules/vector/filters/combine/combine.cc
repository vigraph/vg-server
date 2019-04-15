//==========================================================================
// ViGraph dataflow module: filters/combine/combine.cc
//
// Simple filter which just combines its inputs into a single frame
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"

namespace {

//==========================================================================
// Combine filter
class CombineFilter: public FrameFilter
{
  shared_ptr<Frame> combine{nullptr};

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
void CombineFilter::accept(FramePtr frame)
{
  // If this is the first, just keep it
  if (!combine)
  {
    // Take it over
    combine = frame;
  }
  else
  {
    // Add to existing
    combine->points.insert(combine->points.end(), frame->points.begin(),
                        frame->points.end());
  }
}

//--------------------------------------------------------------------------
// Pre-tick setup
void CombineFilter::pre_tick(const TickData&)
{
  combine.reset();
}

//--------------------------------------------------------------------------
// Post-tick flush
void CombineFilter::post_tick(const TickData& td)
{
  if (!!combine)
  {
    // Reset disparate local timebases to our master
    combine->timestamp = td.t;
    send(combine);
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "combine",
  "Combine",
  "Combine multiple frame inputs into one",
  "vector",
  {}, // no properties
  { { "VectorFrame", true } }, // multiple inputs
  { "VectorFrame" }            // one output
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(CombineFilter, module)
