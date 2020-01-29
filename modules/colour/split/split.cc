//==========================================================================
// ViGraph dataflow module: colour/split/split.cc
//
// Split colour filter
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../colour-module.h"
#include <cmath>

namespace {

//==========================================================================
// Split
class Split: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Split *create_clone() const override
  {
    return new Split{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Input
  Input<Colour::RGB> input;

  // Output
  Output<Number> red;
  Output<Number> green;
  Output<Number> blue;
};

//--------------------------------------------------------------------------
// Tick data
void Split::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(max(red.get_sample_rate(),
                                               max(green.get_sample_rate(),
                                                   blue.get_sample_rate())));
  sample_iterate(td, nsamples, {}, tie(input), tie(red, green, blue),
                 [&](const Colour::RGB& input,
                     Number& red, Number& green, Number& blue)
  {
    red = input.r;
    green = input.g;
    blue = input.b;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "split",
  "Split Colour",
  "colour",
  {},
  {
    { "input",  &Split::input  }
  },
  {
    { "red",   &Split::red },
    { "green", &Split::green },
    { "blue",  &Split::blue }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Split, module)
