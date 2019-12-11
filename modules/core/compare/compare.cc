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

  // Configuration
  Input<Number> value{0.0};

  // Input
  Input<Number> input{0.0};

  // Outputs
  Output<Number> is_lower;     // When input < value
  Output<Number> is_equal;     // When input == value
  Output<Number> is_higher;    // When input > value
  Output<Trigger> went_lower;
  Output<Trigger> went_equal;
  Output<Trigger> went_higher;
};

//--------------------------------------------------------------------------
// Tick
void Compare::tick(const TickData& td)
{
  const auto sample_rate = max(is_lower.get_sample_rate(),
                               max(is_equal.get_sample_rate(),
                                   max(is_higher.get_sample_rate(),
                                       max(went_lower.get_sample_rate(),
                                           max(went_equal.get_sample_rate(),
                                           went_higher.get_sample_rate())))));
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples,
                 {},
                 tie(value, input),
                 tie(is_lower, is_equal, is_higher,
                     went_lower, went_equal, went_higher),
                 [&](Number value, Number input,
                     Number& is_lower, Number& is_equal, Number& is_higher,
                     Trigger& went_lower, Trigger& went_equal,
                     Trigger& went_higher)
  {
    is_lower = is_equal = is_higher = 0.0;
    went_lower = went_equal = went_higher = 0.0;
    if (input > value)
    {
      if (last != Last::higher)
      {
        went_higher = 1;
        last = Last::higher;
      }
      is_higher = 1;
    }
    else if (input < value)
    {
      if (last != Last::lower)
      {
        went_lower = 1;
        last = Last::lower;
      }
      is_lower = 1;
    }
    else
    {
      if (last != Last::equal)
      {
        went_equal = 1;
        last = Last::equal;
      }
      is_equal = 1;
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
  {},
  {
    { "value",        &Compare::value     },
    { "input",        &Compare::input     }
  },
  {
    { "is-lower",     &Compare::is_lower      },
    { "is-equal",     &Compare::is_equal      },
    { "is-higher",    &Compare::is_higher     },
    { "went-lower",   &Compare::went_lower    },
    { "went-equal",   &Compare::went_equal    },
    { "went-higher",  &Compare::went_higher   },
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Compare, module)
