//==========================================================================
// ViGraph dataflow module: bitmap/rgb/rgb.cc
//
// RGB colour filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"
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
  Input<double> r{0.0};
  Input<double> g{0.0};
  Input<double> b{0.0};

  // Input
  Input<Bitmap::Group> input;

  // Output
  Output<Bitmap::Group> output;
};

//--------------------------------------------------------------------------
// Tick data
void RGBColour::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(nsamples, {}, tie(r, g, b, input), tie(output),
                 [&](double r, double g, double b, const Bitmap::Group& input,
                     Bitmap::Group& output)
  {
    output = input;
    Colour::RGB rgb(r, g, b);
    for(auto& item: output.items)
      item.rect.colourise(rgb);
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "rgb",
  "Bitmap RGB",
  "bitmap",
  {},
  {
    { "r",     &RGBColour::r     },
    { "g",     &RGBColour::g     },
    { "b",     &RGBColour::b     },
    { "input", &RGBColour::input }
  },
  {
    { "output", &RGBColour::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(RGBColour, module)
