//==========================================================================
// ViGraph dataflow module: laser/infill-lines/infill-lines.cc
//
// Filter to infill lines with intermediate points
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector/vector-module.h"
#include "vg-laser.h"

namespace {

//==========================================================================
// Add Vertex Repeats
class InfillLines: public SimpleElement
{
private:
  Laser::Optimiser optimiser;

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  InfillLines *create_clone() const override
  {
    return new InfillLines{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<double> max_distance_lit{0.0};
  Setting<double> max_distance_blanked{0.0};

  // Input
  Input<Frame> input;

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Tick data
void InfillLines::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input), tie(output),
                 [&](const Frame& input, Frame& output)
  {
    output.points = optimiser.infill_lines(input.points,
                                           max_distance_lit,
                                           max_distance_blanked);
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "infill-lines",
  "Infill lines",
  "laser",
  {
    { "lit",     &InfillLines::max_distance_lit     },
    { "blanked", &InfillLines::max_distance_blanked }
  },
  {
    { "input", &InfillLines::input }
  },
  {
    { "output", &InfillLines::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(InfillLines, module)
