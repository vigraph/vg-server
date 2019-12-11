//==========================================================================
// ViGraph dataflow module: colour/hsl/hsl.cc
//
// HSL colour filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../colour-module.h"
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

  // Output
  Output<Colour::RGB> output;
};

//--------------------------------------------------------------------------
// Tick data
void HSLColour::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(h, s, l), tie(output),
                 [&](double h, double s, double l,
                     Colour::RGB& output)
  {
    Colour::HSL hsl(h, s, l);
    output = Colour::RGB(hsl);
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "hsl",
  "HSL Colour",
  "colour",
  {},
  {
    { "h", &HSLColour::h },
    { "s", &HSLColour::s },
    { "l", &HSLColour::l },
  },
  {
    { "output", &HSLColour::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(HSLColour, module)
