//==========================================================================
// ViGraph dataflow module: bitmap/twist/twist.cc
//
// Bitmap internal rotation filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"
#include <algorithm>

namespace {

//==========================================================================
// Twist
class Twist: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Twist *create_clone() const override
  {
    return new Twist{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<Number> x{0.0};
  Input<Number> y{0.0};

  // Input
  Input<Bitmap::Group> input;

  // Output
  Output<Bitmap::Group> output;
};

//--------------------------------------------------------------------------
// Tick data
void Twist::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(x, y, input), tie(output),
                 [&](Number x, Number y, const Bitmap::Group& input,
                     Bitmap::Group& output)
  {
    output = input;
    for(auto& item: output.items)
    {
      auto width = item.rect.get_width();
      auto& pixels = item.rect.get_pixels();
      auto amount = static_cast<unsigned int>(x)
                  + static_cast<unsigned int>(y)*width;
      if (amount < pixels.size())
        rotate(pixels.begin(), pixels.begin()+amount, pixels.end());
    }
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "twist",
  "Bitmap twist",
  "bitmap",
  {},
  {
    { "x",     &Twist::x     },
    { "y",     &Twist::y     },
    { "input", &Twist::input }
  },
  {
    { "output", &Twist::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Twist, module)
