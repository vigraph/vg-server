//==========================================================================
// ViGraph dataflow module: vector/translate/translate.cc
//
// Translate filter
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
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
  Input<Frame> input;

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Tick data
void Translate::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(x, y, z, input), tie(output),
                 [&](double x, double y, double z, const Frame& input,
                     Frame& output)
  {
    Vector v(x, y, z);
    output = input;
    for(auto& p: output.points) p+=v;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "translate",
  "Translate",
  "vector",
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
