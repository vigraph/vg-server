//==========================================================================
// ViGraph dataflow module: compare/compare.cc
//
// Compares an input value with a min and max and outputs inside or outside
//
// Copyright (c) 2018-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"

namespace {

//==========================================================================
// Compare control
class Compare: public SimpleElement
{
private:
  enum class Last
  {
    none,
    lower,
    equal,
    higher
  } last{Last::none};

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Compare *create_clone() const override
  {
    return new Compare{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Setting
  Setting<double> on_change;

  // Configuration
  Input<double> value{0.0};

  // Input
  Input<double> input{0.0};

  // Outputs
  Output<double> lower;     // When input < value
  Output<double> equal;     // When input == value
  Output<double> higher;    // When input > value
};

//--------------------------------------------------------------------------
// Tick
void Compare::tick(const TickData& td)
{
  sample_iterate(td.nsamples,
                 tie(on_change),
                 tie(value, input),
                 tie(lower, equal, higher),
                 [&](double on_change,
                     double value, double input,
                     double& lower, double& equal, double& higher)
  {
    lower = equal = higher = 0.0;
    if (input > value)
    {
      if (!on_change || last != Last::higher)
      {
        higher = 1;
        last = Last::higher;
      }
    }
    else if (input < value)
    {
      if (!on_change || last != Last::lower)
      {
        lower = 1;
        last = Last::lower;
      }
    }
    else
    {
      if (!on_change || last != Last::equal)
      {
        equal = 1;
        last = Last::equal;
      }
    }
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "compare",
  "Compare",
  "core",
  {
    { "on-change",  &Compare::on_change }
  },
  {
    { "value",      &Compare::value     },
    { "input",      &Compare::input     }
  },
  {
    { "lower",     &Compare::lower      },
    { "equal",     &Compare::equal      },
    { "higher",    &Compare::higher     }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Compare, module)
