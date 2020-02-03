//==========================================================================
// Sets a bit of a number
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"

namespace {

//==========================================================================
// Set control
class Set: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Set *create_clone() const override
  {
    return new Set{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<Number> bit{0.0};

  // Input
  Input<Number> input{0.0};

  // Outputs
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Tick
void Set::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples,
                 {}, tie(bit, input), tie(output),
                 [&](Number b, Number i, Number& o)
  {
    const auto bitnum = pow(2.0, static_cast<unsigned>(b));
    if (fmod(floor(i / bitnum), 2))
      o = i;
    else
      o = i + bitnum;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "set",
  "Set",
  "binary",
  {},
  {
    { "bit",    &Set::bit     },
    { "input",  &Set::input   },
  },
  {
    { "output", &Set::output  },
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Set, module)
