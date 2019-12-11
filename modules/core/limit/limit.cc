//==========================================================================
// ViGraph dataflow module: limit/limit.cc
//
// Control to limit an input into a fixed range
//
// Copyright (c) 2018-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Limit control
class Limit: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Limit *create_clone() const override
  {
    return new Limit{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<Number> min{0.0};
  Input<Number> max{1.0};

  // Input
  Input<Number> input{0.0};

  // Outputs
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Tick
void Limit::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {},
                 tie(min, max, input),
                 tie(output),
                 [&](double min, double max, double input, double& output)
  {
    if (input < min)
      output = min;
    else if (input > max)
      output = max;
    else
      output = input;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "limit",
  "Limit",
  "core",
  {},
  {
    { "min",   &Limit::min },
    { "max",   &Limit::max },
    { "input", &Limit::input }
  },
  {
    { "output", &Limit::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Limit, module)
