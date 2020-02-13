//==========================================================================
// ViGraph dataflow module: differentiate/differentiate.cc
//
// Module to differentiate an input
//
// Copyright (c) 2018-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Differentiate control
class Differentiate: public SimpleElement
{
private:
  Number last_value{0.0};
  bool last_value_valid{false};

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Differentiate *create_clone() const override
  {
    return new Differentiate{module};
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
void Differentiate::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {},
                 tie(input),
                 tie(output),
                 [&](Number input, Number& output)
  {
    if (last_value_valid)
    {
      output = input - last_value;
    }
    else
    {
      output = 0;  // Prevent spikes at start
      last_value_valid = true;
    }

    last_value = input;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "differentiate",
  "Differentiate",
  "maths",
  {},
  {
    { "input", &Differentiate::input }
  },
  {
    { "output", &Differentiate::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Differentiate, module)
