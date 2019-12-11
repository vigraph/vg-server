//==========================================================================
// ViGraph dataflow module: laser/reorder-segments/reorder-segments.cc
//
// Filter to show blanked points

// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector/vector-module.h"
#include "vg-laser.h"

namespace {

//==========================================================================
// Reorder Segments
class ReorderSegments: public SimpleElement
{
private:
  Laser::Optimiser optimiser;

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  ReorderSegments *create_clone() const override
  {
    return new ReorderSegments{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Input
  Input<Frame> input;

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Tick data
void ReorderSegments::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input), tie(output),
                 [&](const Frame& input, Frame& output)
  {
    output.points = optimiser.reorder_segments(input.points);
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "reorder-segments",
  "Reorder segments",
  "laser",
  {},
  {
    { "input", &ReorderSegments::input }
  },
  {
    { "output", &ReorderSegments::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ReorderSegments, module)
