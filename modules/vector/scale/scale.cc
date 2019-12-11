//==========================================================================
// ViGraph dataflow module: vector/scale/scale.cc
//
// Scale filter
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include <cmath>

namespace {

//==========================================================================
// Scale
class Scale: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Scale *create_clone() const override
  {
    return new Scale{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<double> x{1.0};
  Input<double> y{1.0};
  Input<double> z{1.0};

  // Input
  Input<Frame> input;

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Tick data
void Scale::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(x, y, z, input), tie(output),
                 [&](double x, double y, double z, const Frame& input,
                     Frame& output)
  {
    output = input;
    for(auto& p: output.points)
    {
      p.x *= x;
      p.y *= y;
      p.z *= z;
    }
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "scale",
  "Scale",
  "vector",
  {},
  {
    { "x",     &Scale::x     },
    { "y",     &Scale::y     },
    { "z",     &Scale::z     },
    { "input", &Scale::input }
  },
  {
    { "output", &Scale::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Scale, module)
