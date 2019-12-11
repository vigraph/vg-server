//==========================================================================
// ViGraph dataflow module: core/beat/beat.cc
//
// Control to provide repetitive triggers at a defined interval
//
// Copyright (c) 2018-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"
#include <cmath>
#include <stdlib.h>

const double interval_rounding = 1e-6;  // Fix rounding when very near

namespace {

//==========================================================================
// Beat control
class Beat: public SimpleElement
{
 private:
  // Dynamic state
  bool active{false};
  bool synchronised{false};
  Dataflow::timestamp_t last_trigger{0.0};

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Beat *create_clone() const override
  {
    return new Beat{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<Number> interval{0.0};
  Input<Number> offset{0.0};

  // Triggers
  Input<Trigger> start{0.0};    // Trigger to start
  Input<Trigger> stop{0.0};     // Trigger to stop

  // Outputs
  Output<Trigger> output;       // Trigger output
};

//--------------------------------------------------------------------------
// Tick
void Beat::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();
  auto sample_time = td.first_sample_at(sample_rate);
  const auto sample_duration = td.sample_duration(sample_rate);
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {},
                 tie(interval, offset, start, stop),
                 tie(output),
                 [&](double interval, double offset,
                     Trigger _start, Trigger _stop,
                     Trigger& output)
  {
    if (_stop)
    {
      active = false;
      synchronised = false;
    }
    else if (_start || !start.connected())
    {
      active = true;
    }

    output = 0;

    if (active)
    {
      if (!synchronised)
      {
        last_trigger = sample_time-interval;
        synchronised = true;
      }

      if (sample_time >= last_trigger + interval + offset - interval_rounding)
      {
        last_trigger += interval;       // Maintain constant timebase
        output = 1;
      }
    }
    sample_time += sample_duration;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "beat",
  "Beat",
  "core",
  {},
  {
    { "interval",  &Beat::interval },
    { "offset",    &Beat::offset   },
    { "start",     &Beat::start    },
    { "stop",      &Beat::stop     }
  },
  {
    { "output",    &Beat::output   }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Beat, module)
