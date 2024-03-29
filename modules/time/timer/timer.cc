//==========================================================================
// ViGraph dataflow module: time/timer/timer.cc
//
// Control to provide a retriggerable monostable
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
  bool is_active{false};
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

  // Configuration
  Input<Number> period{0.0};   // Period to stay active

  // Triggers
  Input<Trigger> start{0};    // Trigger to start
  Input<Trigger> reset{0};    // Stop and reset

  // Outputs
  Output<Trigger> finished;     // Trigger output on completion
  Output<Number> active;       // Level output
};

//--------------------------------------------------------------------------
// Tick
void Timer::tick(const TickData& td)
{
  const auto sample_rate = max(finished.get_sample_rate(),
                               active.get_sample_rate());
  auto sample_time = td.first_sample_at(sample_rate);
  const auto sample_duration = td.sample_duration(sample_rate);
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {},
                 tie(period, start, reset),
                 tie(finished, active),
                 [&](Number period, Trigger _start, Trigger _reset,
                     Trigger& finished, Number& active)
  {
    if (_reset)
    {
      is_active = false;
    }
    else if (_start)
    {
      is_active = true;
      start_time = sample_time;
    }

    if (is_active && sample_time-start_time >= period)
    {
      finished = 1;
      is_active = false;
    }
    else finished = 0;

    active = is_active?1:0;
    sample_time += sample_duration;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "timer",
  "Timer",
  "time",
  {},
  {
    { "period",    &Timer::period   },
    { "start",     &Timer::start    },
    { "reset",     &Timer::reset    }
  },
  {
    { "finished",  &Timer::finished },
    { "active",    &Timer::active   }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Timer, module)
