//==========================================================================
// ViGraph dataflow module: integrate/integrate.cc
//
// Module to integrate an input
//
// Copyright (c) 2018-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Integrate control
class Integrate: public SimpleElement
{
private:
  Number integral{0.0};

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Integrate *create_clone() const override
  {
    return new Integrate{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Input
  Input<Number> input{0.0};

  // Outputs
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Tick
void Integrate::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {},
                 tie(input),
                 tie(output),
                 [&](Number input, Number& output)
  {
    integral += input;
    output = integral;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "integrate",
  "Integrate",
  "maths",
  {},
  {
    { "input", &Integrate::input }
  },
  {
    { "output", &Integrate::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Integrate, module)
