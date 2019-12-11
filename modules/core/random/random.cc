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
  Input<Number> min{0.0};
  Input<Number> max{1.0};

  // Triggers
  Input<Trigger> trigger{0.0};

  // Outputs
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Tick
void Random::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {},
                 tie(min, max, trigger),
                 tie(output),
                 [&](double min, double max, Trigger _trigger, double& output)
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
