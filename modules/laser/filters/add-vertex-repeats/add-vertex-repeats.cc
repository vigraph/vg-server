//==========================================================================
// ViGraph dataflow module:
//  laser/filters/add-vertex-repeats/add-vertex-repeats.cc
//
// Adds repeated points at sharp vertices
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../vector/vector-module.h"
#include "vg-laser.h"
#include <cmath>

namespace {

const double default_repeats = 3;
const double default_max_angle = 30;

//==========================================================================
// AddVertexRepeats filter
class AddVertexRepeatsFilter: public FrameFilter
{
public:
  double max_angle = default_max_angle;  // in degrees
  int repeats = default_repeats;

private:
  Laser::Optimiser optimiser;

  // Filter/Element virtuals
  void accept(FramePtr frame) override;

public:
  using FrameFilter::FrameFilter;
};

//--------------------------------------------------------------------------
// Process some data
void AddVertexRepeatsFilter::accept(FramePtr frame)
{
  frame->points = optimiser.add_vertex_repeats(frame->points,
                                               max_angle*pi/180, // to radians
                                               repeats);
  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "add-vertex-repeats",
  "Add vertex repeats",
  "Add additional points at sharp vertices for laser scanners",
  "laser",
  {
    { "repeats", { "Number of points to add at a vertex", Value::Type::number,
                   &AddVertexRepeatsFilter::repeats, true } },
    { "max-angle", { "Maximum turn angle at a vertex before adding points",
                     Value::Type::number,
                     &AddVertexRepeatsFilter::max_angle, true } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AddVertexRepeatsFilter, module)
