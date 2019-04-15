//==========================================================================
// ViGraph dataflow module: laser/filters/reorder-segments/reorder-segments.cc
//
// Reorder segments to optimise laser scanning
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../vector/vector-module.h"
#include "vg-laser.h"
#include <cmath>

namespace {

//==========================================================================
// ReorderSegments filter
class ReorderSegmentsFilter: public FrameFilter
{
  Laser::Optimiser optimiser;

  // Filter/Element virtuals
  void accept(FramePtr frame) override;

public:
  using FrameFilter::FrameFilter;
};

//--------------------------------------------------------------------------
// Process some data
void ReorderSegmentsFilter::accept(FramePtr frame)
{
  frame->points = optimiser.reorder_segments(frame->points);
  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "reorder-segments",
  "Reorder segments",
  "Re-orders segments between blanks to optimise laser scanner movement",
  "laser",
  {},  // no properties
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ReorderSegmentsFilter, module)
