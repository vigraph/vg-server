//==========================================================================
// ViGraph dataflow module: core/interpolator/interpolator.cc
//
// Linear interpolator
//
// Copyright (c) 2018-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"
#include <cmath>
#include <stdlib.h>

namespace {

//==========================================================================
// Interpolator control
class Interpolator: public SimpleElement
{
 private:
  // Dynamic state
  bool active{false};
  Dataflow::timestamp_t start_time{0.0};

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Interpolator *create_clone() const override
  {
    return new Interpolator{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<Number> from{0.0};
  Input<Number> to{1.0};
  Input<Number> period{0.0};

  // Triggers
  Input<Trigger> start{0};    // Trigger to start

  // Outputs
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Tick
void Interpolator::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();
  auto sample_time = td.first_sample_at(sample_rate);
  const auto sample_duration = td.sample_duration(sample_rate);
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {},
                 tie(from, to, period, start),
                 tie(output),
                 [&](Number from, Number to, Number period,
                     Trigger _start,
                     Number& output)
  {
    if (_start || (!active && !start.connected()))
    {
      start_time = sample_time;
      active = true;
    }

    output = from;

    if (active && period > 0.0)
    {
      auto t = sample_time - start_time;
      output += (to-from)*min(t/period, 1.0);  // Fence at end
    }
    sample_time += sample_duration;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "interpolator",
  "Interpolator",
  "core",
  {},
  {
    { "from",   &Interpolator::from   },
    { "to",     &Interpolator::to     },
    { "period", &Interpolator::period },
    { "start",  &Interpolator::start  }
  },
  {
    { "output",    &Interpolator::output   }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Interpolator, module)
