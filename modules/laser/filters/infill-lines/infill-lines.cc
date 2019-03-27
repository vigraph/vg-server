//==========================================================================
// ViGraph dataflow module: laser/filters/infill-lines/infill-lines.cc
//
// Fills in lines to get constant brightness on laser scanning
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../vector/vector-module.h"
#include "vg-laser.h"
#include <cmath>

namespace {

//==========================================================================
// InfillLines filter
class InfillLinesFilter: public FrameFilter
{
public:
  double max_distance_lit = 0;
  double max_distance_blanked = 0;

private:
  Laser::Optimiser optimiser;

  // Filter/Element virtuals
  void accept(FramePtr frame) override;

public:
  using FrameFilter::FrameFilter;
};

//--------------------------------------------------------------------------
// Process some data
void InfillLinesFilter::accept(FramePtr frame)
{
  frame->points = optimiser.infill_lines(frame->points, max_distance_lit,
                                         max_distance_blanked);
  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "infill-lines",
  "Infill lines",
  "Adds a spread of points to lines to get constant brightness in laser scans",
  "laser",
  {
    { "lit", { "Maximum distance between lit points", Value::Type::number,
      static_cast<double Element::*>(&InfillLinesFilter::max_distance_lit),
      true } },
    { "blanked", { "Maximum distance between blanked points",
      Value::Type::number,
      static_cast<double Element::*>(&InfillLinesFilter::max_distance_blanked),
      true } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(InfillLinesFilter, module)
