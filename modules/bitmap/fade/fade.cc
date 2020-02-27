//==========================================================================
// ViGraph dataflow module: bitmap/fade/fade.cc
//
// Fade filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"
#include <cmath>

namespace {

//==========================================================================
// Fade
class Fade: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Fade *create_clone() const override
  {
    return new Fade{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<Number> alpha{1.0};

  // Input
  Input<Bitmap::Group> input;

  // Output
  Output<Bitmap::Group> output;
};

//--------------------------------------------------------------------------
// Tick data
void Fade::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(alpha, input), tie(output),
                 [&](Number alpha, const Bitmap::Group& input,
                     Bitmap::Group& output)
  {
    output = input;
    for(auto& item: output.items)
      item.rect.fade(alpha);
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "fade",
  "Bitmap fade",
  "bitmap",
  {},
  {
    { "alpha", &Fade::alpha },
    { "input", &Fade::input }
  },
  {
    { "output", &Fade::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Fade, module)
