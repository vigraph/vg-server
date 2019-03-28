//==========================================================================
// ViGraph dataflow module: laser/filters/beamify/beamify.cc
//
// Filter which replicates points to create beams within a figure
//
// <beamify span="10" multiple="5"/>
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../vector/vector-module.h"

namespace {

//==========================================================================
// Beamify filter
class BeamifyFilter: public FrameFilter
{
public:
  int span = 0;      // Number of points between beams
  int n = 0;         // Number of points to replicate in the beam

  // Filter/Element virtuals
  void accept(FramePtr frame) override;

public:
  using FrameFilter::FrameFilter;
};

//--------------------------------------------------------------------------
// Process some data
void BeamifyFilter::accept(FramePtr frame)
{
  // Insert 'n' points every 'span'
  if (span) for(size_t i=0; i<frame->points.size(); i+=span+n)
    frame->points.insert(frame->points.begin()+i, n, frame->points[i]);
  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "beamify",
  "Beamify",
  "Add bright points to a continuous frame",
  "vector",
  {
    { "span", { "Distance between points", Value::Type::number,
               static_cast<int Element::*>(&BeamifyFilter::span), true}},
    { "n",    { "Number of points to add", Value::Type::number,
               static_cast<int Element::*>(&BeamifyFilter::n), true}},
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(BeamifyFilter, module)
