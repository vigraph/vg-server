//==========================================================================
// ViGraph dataflow module: laser/add-blanking-anchors/add-blanking-anchors.cc
//
// Filter to show blanked points

// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector/vector-module.h"
#include "vg-laser.h"

namespace {

const int default_leading = 5;
const int default_trailing = 5;

//==========================================================================
// Add Blanking Anchors
class AddBlankingAnchors: public SimpleElement
{
private:
  Laser::Optimiser optimiser;

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  AddBlankingAnchors *create_clone() const override
  {
    return new AddBlankingAnchors{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<Integer> leading{default_leading};
  Setting<Integer> trailing{default_trailing};

  // Input
  Input<Frame> input;

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Tick data
void AddBlankingAnchors::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input), tie(output),
                 [&](const Frame& input, Frame& output)
  {
    output.points = optimiser.add_blanking_anchors(input.points,
                                                   leading, trailing);
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "add-blanking-anchors",
  "Add blanking anchors",
  "laser",
  {
    { "leading",  &AddBlankingAnchors::leading  },
    { "trailing", &AddBlankingAnchors::trailing }
  },
  {
    { "input", &AddBlankingAnchors::input }
  },
  {
    { "output", &AddBlankingAnchors::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AddBlankingAnchors, module)
