//==========================================================================
// ViGraph dataflow module: laser/beamify/beamify.cc
//
// Filter to add points to create beams within a figure
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector/vector-module.h"

namespace {

//==========================================================================
// Show Blanking
class Beamify: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Beamify *create_clone() const override
  {
    return new Beamify{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Setting<Integer> every;
  Setting<Integer> extra;

  // Input
  Input<Frame> input;

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Tick data
void Beamify::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input), tie(output),
                 [&](const Frame& input, Frame& output)
  {
    output = input;

    // Insert 'extra' points every 'every'
    if (every)
    {
      for(size_t i=0; i<output.points.size(); i+=every+extra)
        output.points.insert(output.points.begin()+i, extra, output.points[i]);
    }
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "beamify",
  "Beamify",
  "laser",
  {
    { "every", &Beamify::every },
    { "extra", &Beamify::extra }
  },
  {
    { "input", &Beamify::input }
  },
  {
    { "output", &Beamify::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Beamify, module)
