//==========================================================================
// ViGraph dataflow module: vector/fade/fade.cc
//
// Vector fade filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
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
  Input<double> alpha{1.0};

  // Input
  Input<Frame> input;

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Tick data
void Fade::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(alpha, input), tie(output),
                 [&](double alpha, const Frame& input, Frame& output)
  {
    output = input;

    for(auto& p: output.points)
      if (!p.is_blanked()) p.c.fade(alpha);
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "fade",
  "Vector fade",
  "vector",
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
