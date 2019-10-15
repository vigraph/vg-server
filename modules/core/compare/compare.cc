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
    inside,
    outside
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
  Input<double> min{0.0};
  Input<double> max{1.0};

  // Input
  Input<double> input{0.0};

  // Outputs
  Output<double> inside;     // When inside the range (inclusive)
  Output<double> outside;    // When outside the range (exclusive)
};

//--------------------------------------------------------------------------
// Tick
void Compare::tick(const TickData& td)
{
  sample_iterate(td.nsamples,
                 tie(on_change),
                 tie(min, max, input),
                 tie(inside, outside),
                 [&](double on_change,
                     double min, double max, double input,
                     double& inside, double& outside)
  {
    inside = outside = 0.0;
    if (input > max || input < min)
    {
      if (!on_change || last != Last::outside)
      {
        outside = 1;
        last = Last::outside;
      }
    }
    else
    {
      if (!on_change || last != Last::inside)
      {
        inside = 1;
        last = Last::inside;
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
    { "min",        &Compare::min       },
    { "max",        &Compare::max       },
    { "input",      &Compare::input     }
  },
  {
    { "inside",     &Compare::inside    },
    { "outside",    &Compare::outside   }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Compare, module)
