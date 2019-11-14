//==========================================================================
// ViGraph dataflow module: vector/hsl/hsl.cc
//
// HSL colour filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
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
  Input<Frame> input;

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Tick data
void HSLColour::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(nsamples, {}, tie(h, s, l, input), tie(output),
                 [&](double h, double s, double l, const Frame& input,
                     Frame& output)
  {
    output = input;
    Colour::HSL hsl(h, s, l);
    Colour::RGB rgb(hsl);
    for(auto& p: output.points)
      if (!p.is_blanked()) p.c = rgb;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "hsl",
  "Vector HSL",
  "vector",
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
