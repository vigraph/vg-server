//==========================================================================
// ViGraph dataflow module: random/random.cc
//
// Control to generate random values
//
// Copyright (c) 2018-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Random control
class Random: public SimpleElement
{
private:
  double current{0.0};

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Random *create_clone() const override
  {
    return new Random{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<double> min{0.0};
  Input<double> max{1.0};

  // Triggers
  Input<double> trigger{0.0};

  // Outputs
  Output<double> output;
};

//--------------------------------------------------------------------------
// Tick
void Random::tick(const TickData& td)
{
  sample_iterate(td.nsamples, {},
                 tie(min, max, trigger),
                 tie(output),
                 [&](double min, double max, double _trigger, double& output)
  {
    if (_trigger || !trigger.connected())
      current = min + (double)rand()/RAND_MAX*(max-min);

    output = current;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "random",
  "Random",
  "core",
  {},
  {
    { "min",      &Random::min },
    { "max",      &Random::max },
    { "trigger",  &Random::trigger }
  },
  {
    { "output", &Random::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Random, module)
