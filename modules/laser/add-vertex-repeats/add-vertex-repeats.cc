//==========================================================================
// ViGraph dataflow module: laser/add-vertex-repeats/add-vertex-repeats.cc
//
// Filter to show blanked points

// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector/vector-module.h"
#include "vg-laser.h"

namespace {

const auto default_repeats = 3;
const auto default_max_angle = 30.0;

//==========================================================================
// Add Vertex Repeats
class AddVertexRepeats: public SimpleElement
{
private:
  Laser::Optimiser optimiser;

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  AddVertexRepeats *create_clone() const override
  {
    return new AddVertexRepeats{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<Integer> repeats{default_repeats};
  Setting<Number> max_angle{default_max_angle};

  // Input
  Input<Frame> input;

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Tick data
void AddVertexRepeats::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input), tie(output),
                 [&](const Frame& input, Frame& output)
  {
    output.points = optimiser.add_vertex_repeats(input.points,
                                                 max_angle*pi/180,
                                                 repeats);
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "add-vertex-repeats",
  "Add vertex repeats",
  "laser",
  {
    { "repeats",   &AddVertexRepeats::repeats   },
    { "max-angle", &AddVertexRepeats::max_angle }
  },
  {
    { "input", &AddVertexRepeats::input }
  },
  {
    { "output", &AddVertexRepeats::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AddVertexRepeats, module)
