//==========================================================================
// ViGraph dataflow module: bitmap/translate/translate.cc
//
// Translate filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"
#include <cmath>

namespace {

//==========================================================================
// Translate
class Translate: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Translate *create_clone() const override
  {
    return new Translate{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<double> x{0.0};
  Input<double> y{0.0};
  Input<double> z{0.0};

  // Input
  Input<Bitmap::Group> input;

  // Output
  Output<Bitmap::Group> output;
};

//--------------------------------------------------------------------------
// Tick data
void Translate::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(nsamples, {}, tie(x, y, z, input), tie(output),
                 [&](double x, double y, double z, const Bitmap::Group& input,
                     Bitmap::Group& output)
  {
    Vector v(x, y, z);
    output = input;
    for(auto& item: output.items)
      item.pos += v;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "translate",
  "Bitmap translate",
  "bitmap",
  {},
  {
    { "x",     &Translate::x     },
    { "y",     &Translate::y     },
    { "z",     &Translate::z     },
    { "input", &Translate::input }
  },
  {
    { "output", &Translate::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Translate, module)
