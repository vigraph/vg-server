//==========================================================================
// ViGraph dataflow module: bitmap/hsl/hsl.cc
//
// HSL colour filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"
#include <cmath>

namespace {

//==========================================================================
// HSLColour
class HSLColour: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  HSLColour *create_clone() const override
  {
    return new HSLColour{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<double> h{0.0};
  Input<double> s{1.0};
  Input<double> l{0.5};

  // Input
  Input<Bitmap::Group> input;

  // Output
  Output<Bitmap::Group> output;
};

//--------------------------------------------------------------------------
// Tick data
void HSLColour::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(nsamples, {}, tie(h, s, l, input), tie(output),
                 [&](double h, double s, double l, const Bitmap::Group& _input,
                     Bitmap::Group& output)
  {
    if (input.connected())
      output = _input;
    else
    {
      Bitmap::Rectangle r(1,1);
      r.fill(Colour::black); // Opaque
      output.add(r);
    }

    Colour::HSL hsl(h, s, l);
    Colour::RGB rgb(hsl);
    for(auto& item: output.items)
      item.rect.colourise(rgb);
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "hsl",
  "Bitmap HSL",
  "bitmap",
  {},
  {
    { "h",     &HSLColour::h     },
    { "s",     &HSLColour::s     },
    { "l",     &HSLColour::l     },
    { "input", &HSLColour::input }
  },
  {
    { "output", &HSLColour::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(HSLColour, module)
