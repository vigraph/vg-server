//==========================================================================
// ViGraph dataflow module: core/clock/clock.cc
//
// Control to provide a resettable timebase output
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"

namespace {

//==========================================================================
// Clock control
class Clock: public SimpleElement
{
 private:
  // Dynamic state
  bool active{false};
  Dataflow::timestamp_t start_time{0.0};

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Clock *create_clone() const override
  {
    return new Clock{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Triggers
  Input<Trigger> start{0.0};    // Trigger to start
  Input<Trigger> stop{0.0};     // Trigger to stop

  // Outputs
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Tick
void Clock::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();
  auto sample_time = td.first_sample_at(sample_rate);
  const auto sample_duration = td.sample_duration(sample_rate);
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {},
                 tie(start, stop),
                 tie(output),
                 [&](Trigger _start, Trigger _stop,
                     double& output)
  {
    if (_stop)
    {
      active = false;
    }
    else if (_start || (!active && !start.connected())) // Freerun only once
    {
      active = true;
      start_time = sample_time;
    }

    if (active)
      output = sample_time - start_time;
    else
      output = 0;

    sample_time += sample_duration;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "clock",
  "Clock",
  "core",
  {},
  {
    { "start",     &Clock::start    },
    { "stop",      &Clock::stop     }
  },
  {
    { "output",    &Clock::output   }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Clock, module)
