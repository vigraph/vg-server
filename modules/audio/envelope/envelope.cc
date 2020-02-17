//==========================================================================
// ViGraph dataflow module: envelope/envelope.cc
//
// Control to output an ADSR envelope value on trigger/release
//
// envelope attack="0.0" decay="0.0" sustain="1.0" release="0.0" ...
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"
#include <iostream>

namespace {

//==========================================================================
// Envelope control
class Envelope: public SimpleElement
{
private:
  // Dynamic state
  enum class State
  {
    off,
    attack,
    decay,
    sustain,
    release
  } state = State::off;
  timestamp_t state_changed_time{0};
  Number release_start_value{0};
  Number attack_start_value{0};

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Envelope *create_clone() const override
  {
    return new Envelope{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<Number> attack{0.0};   // Time for attack
  Input<Number> decay{0.0};    // Time for decay
  Input<Number> sustain{1.0};  // Value to sustain at
  Input<Number> release{0.0};  // Time to release

  // Triggers
  Input<Trigger> start{0};   // Trigger to start attack
  Input<Trigger> stop{0};    // Trigger to start release
  Input<Trigger> reset{0};   // Hard reset

  // Outputs
  Output<Number> output;       // Value output
  Output<Trigger> finished;    // Trigger when fully released
};

//--------------------------------------------------------------------------
// Tick
void Envelope::tick(const TickData& td)
{
  const auto sample_rate = max(output.get_sample_rate(),
                               finished.get_sample_rate());
  auto sample_time = td.first_sample_at(sample_rate);
  const auto sample_duration = td.sample_duration(sample_rate);
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {},
                 tie(attack, decay, sustain, release, start, stop, reset),
                 tie(output, finished),
                 [&](Number attack, Number decay,
                     Number sustain, Number release,
                     Trigger start, Trigger stop, Trigger reset,
                     Number& output, Trigger& finished)
  {
    if (reset)
    {
      state = State::off;
    }
    else if (stop)
    {
      switch (state)
      {
        case State::attack:
        case State::decay:
        case State::sustain:
          state = State::release;
          state_changed_time = sample_time;
          break;

        default:;
      }
    }
    else if (start)
    {
      state = State::attack; // Note whatever previous state was, to restart
      state_changed_time = sample_time;
    }

    finished = 0;  // By default

    auto delta = sample_time-state_changed_time;
    switch (state)
    {
      case State::off:
        attack_start_value = release_start_value = 0;
        output = 0.0;
        break;

      case State::attack:
        if (delta < attack)
        {
          release_start_value = output =
            attack_start_value + (1-attack_start_value)*delta/attack;
          break;
        }
        state = State::decay;
        state_changed_time = sample_time;
        delta = 0;
        release_start_value = output = 1.0;
        // Fall through

      case State::decay:
        if (decay < delta)
        {
          release_start_value = attack_start_value = output =
            1.0-(1.0-sustain)*delta/decay;
          break;
        }
        state = State::sustain;
        state_changed_time = sample_time;
        // Fall through

      case State::sustain:
        attack_start_value = release_start_value = sustain;
        output = sustain;
      break;

      case State::release:
        if (delta >= release)
        {
          state = State::off;
          attack_start_value = output = 0.0;
          finished = 1;
        }
        else
        {
          attack_start_value = output =
            release_start_value*(1.0-delta/release);
        }
      break;
    }
    sample_time += sample_duration;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "envelope",
  "Envelope",
  "audio",
  {},
  {
    { "attack",    &Envelope::attack    },
    { "decay",     &Envelope::decay     },
    { "sustain",   &Envelope::sustain   },
    { "release",   &Envelope::release   },
    { "start",     &Envelope::start     },
    { "stop",      &Envelope::stop      },
    { "reset",     &Envelope::reset     }
  },
  {
    { "output",    &Envelope::output    },
    { "finished",  &Envelope::finished  }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Envelope, module)
