//==========================================================================
// ViGraph dataflow module: bitmap/fill/fill.cc
//
// Fill with colour filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"
#include "../../colour/colour-module.h"

namespace {

//==========================================================================
// FillFilter
class FillFilter: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  FillFilter *create_clone() const override
  {
    return new FillFilter{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<Colour::RGB> colour{Colour::white};

  // Input
  Input<Bitmap::Group> input;

  // Output
  Output<Bitmap::Group> output;
};

//--------------------------------------------------------------------------
// Tick data
void FillFilter::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(nsamples, {}, tie(colour, input), tie(output),
                 [&](const Colour::RGB& colour, const Bitmap::Group& input,
                     Bitmap::Group& output)
  {
    output = input;
    for(auto& item: output.items)
      item.rect.colourise(colour);
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "fill",
  "Bitmap fill",
  "bitmap",
  {},
  {
    { "colour", &FillFilter::colour },
    { "input",  &FillFilter::input  }
  },
  {
    { "output", &FillFilter::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(FillFilter, module)
