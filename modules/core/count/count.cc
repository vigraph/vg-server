//==========================================================================
// ViGraph dataflow module: count/count.cc
//
// Control to count with a delta on each tick or trigger
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"
#include <cmath>
#include <stdlib.h>

namespace {

//==========================================================================
// Count
class Count: public SimpleElement
{
  Number value{0.0};

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Count *create_clone() const override
  {
    return new Count{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Triggers
  Input<Number> delta{1.0};
  Input<Number> up{0.0};
  Input<Number> down{0.0};
  Input<Number> reset{0.0};

  // Outputs
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Tick
void Count::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {},
                 tie(delta, up, down, reset),
                 tie(output),
                 [&](Number delta,
                     Number up, Number down, Number reset,
                     Number& output)
  {
    if (reset)
      value = 0.0;
    else
      value += (up-down)*delta;

    output = value;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "count",
  "Count",
  "core",
  {},
  {
    { "delta",    &Count::delta   },
    { "up",       &Count::up      },
    { "down",     &Count::down    },
    { "reset",    &Count::reset   }
  },
  {
    { "output",   &Count::output  }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Count, module)
