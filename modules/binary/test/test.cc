//==========================================================================
// Tests if a bit is set in a number
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"

namespace {

//==========================================================================
// Test control
class Test: public SimpleElement
{
private:
  bool last = false;

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Test *create_clone() const override
  {
    return new Test{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<Number> bit{0.0};

  // Input
  Input<Number> input{0.0};

  // Outputs
  Output<Number> is_set;
  Output<Trigger> went_set;
  Output<Trigger> went_unset;
};

//--------------------------------------------------------------------------
// Tick
void Test::tick(const TickData& td)
{
  const auto sample_rate = max(is_set.get_sample_rate(),
                               max(went_set.get_sample_rate(),
                                   went_unset.get_sample_rate()));
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples,
                 {}, tie(bit, input), tie(is_set, went_set, went_unset),
                 [&](Number b, Number i, Number& is, Trigger& ws, Trigger& wu)
  {
    is = fmod(floor(i / pow(2.0, static_cast<unsigned>(b))), 2);
    ws = !last && is;
    wu = last && !is;
    last = is;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "test",
  "Test",
  "binary",
  {},
  {
    { "bit",        &Test::bit        },
    { "input",      &Test::input      },
  },
  {
    { "is-set",     &Test::is_set     },
    { "went-set",   &Test::went_set   },
    { "went-unset", &Test::went_unset },
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Test, module)
