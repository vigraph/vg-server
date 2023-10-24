//==========================================================================
// ViGraph dataflow module: laser/strip-blank-runs/strip-blank-runs.cc
//
// Filter to strip blank runs with intermediate points
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector/vector-module.h"
#include "vg-laser.h"

namespace {

//==========================================================================
// Add Vertex Repeats
class StripBlankRuns: public SimpleElement
{
private:
  Laser::Optimiser optimiser;

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  StripBlankRuns *create_clone() const override
  {
    return new StripBlankRuns{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<Number> threshold{5.0};

  // Input
  Input<Frame> input;

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Tick data
void StripBlankRuns::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input), tie(output),
                 [&](const Frame& input, Frame& output)
  {
    output.points = optimiser.strip_blank_runs(input.points, threshold);
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "strip-blank-runs",
  "Strip runs of blanks",
  "laser",
  {
    { "threshold", &StripBlankRuns::threshold }
  },
  {
    { "input", &StripBlankRuns::input }
  },
  {
    { "output", &StripBlankRuns::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(StripBlankRuns, module)
