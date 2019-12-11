//==========================================================================
// ViGraph dataflow module: colour/rgb/rgb.cc
//
// RGB colour filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../colour-module.h"
#include <cmath>

namespace {

//==========================================================================
// RGBColour
class RGBColour: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  RGBColour *create_clone() const override
  {
    return new RGBColour{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<Number> r{0.0};
  Input<Number> g{0.0};
  Input<Number> b{0.0};

  // Output
  Output<Colour::RGB> output;
};

//--------------------------------------------------------------------------
// Tick data
void RGBColour::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(r, g, b), tie(output),
                 [&](double r, double g, double b,
                     Colour::RGB& output)
  {
    output = Colour::RGB(r, g, b);
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "rgb",
  "RGB Colour",
  "colour",
  {},
  {
    { "r", &RGBColour::r },
    { "g", &RGBColour::g },
    { "b", &RGBColour::b },
  },
  {
    { "output", &RGBColour::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(RGBColour, module)
