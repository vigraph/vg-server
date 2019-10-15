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
  Dataflow::timestamp_t last_trigger{-1000000.0};

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
  Input<double> interval{0.0};

  // Triggers
  Input<double> start{0.0};    // Trigger to start
  Input<double> stop{0.0};     // Trigger to stop

  // Outputs
  Output<double> output;       // Trigger output
};

//--------------------------------------------------------------------------
// Tick
void Beat::tick(const TickData& td)
{
  auto count{0};
  sample_iterate(td.nsamples, {},
                 tie(interval, start, stop),
                 tie(output),
                 [&](double interval,
                     double _start, double _stop,
                     double& output)
  {
    if (_stop)
    {
      active = false;
      last_trigger = -1000000.0;  // restart timebase
    }
    else if (_start || !start.connected())
    {
      active = true;
    }

    const auto sample_time = td.timestamp + count++/td.sample_rate;
    if (active && sample_time >= last_trigger + interval - interval_rounding)
    {
      if (last_trigger < 0.0)
        last_trigger = sample_time;     // Reset to now
      else
        last_trigger += interval;       // Maintain constant timebase

      output = 1;
    }
    else output = 0;
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
    { "start",     &Beat::start    },
    { "stop",      &Beat::stop     }
  },
  {
    { "output",    &Beat::output   }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Beat, module)
