//==========================================================================
// ViGraph dataflow module: core/timer/timer.cc
//
// Control to provide a resettable timebase output
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"

namespace {

//==========================================================================
// Timer control
class Timer: public SimpleElement
{
 private:
  // Dynamic state
  bool active{false};
  Dataflow::timestamp_t start_time{0.0};

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Timer *create_clone() const override
  {
    return new Timer{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Triggers
  Input<double> start{0.0};    // Trigger to start
  Input<double> stop{0.0};     // Trigger to stop

  // Outputs
  Output<double> output;       // Trigger output
};

//--------------------------------------------------------------------------
// Tick
void Timer::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();
  auto sample_time = td.first_sample_at(sample_rate);
  const auto sample_duration = td.sample_duration(sample_rate);
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(nsamples, {},
                 tie(start, stop),
                 tie(output),
                 [&](double _start, double _stop,
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
  "timer",
  "Timer",
  "core",
  {},
  {
    { "start",     &Timer::start    },
    { "stop",      &Timer::stop     }
  },
  {
    { "output",    &Timer::output   }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Timer, module)
