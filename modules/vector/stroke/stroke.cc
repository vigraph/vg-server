//==========================================================================
// ViGraph dataflow module: vector/stroke/stroke.cc
//
// Stroke colour filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include "../../colour/colour-module.h"

namespace {

//==========================================================================
// StrokeColour
class StrokeColour: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  StrokeColour *create_clone() const override
  {
    return new StrokeColour{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<Colour::RGB> colour{Colour::white};

  // Input
  Input<Frame> input;

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Tick data
void StrokeColour::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(colour, input), tie(output),
                 [&](const Colour::RGB& colour, const Frame& input,
                     Frame& output)
  {
    output = input;
    for(auto& p: output.points)
      if (!p.is_blanked()) p.c = colour;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "stroke",
  "Vector Stroke",
  "vector",
  {},
  {
    { "colour", &StrokeColour::colour },
    { "input",  &StrokeColour::input  }
  },
  {
    { "output", &StrokeColour::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(StrokeColour, module)
