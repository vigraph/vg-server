//==========================================================================
// ViGraph dataflow module: time/now/now.cc
//
// Outputs current time / date
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"

namespace {

//==========================================================================
// Now control
class Now: public SimpleElement
{
 private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Now *create_clone() const override
  {
    return new Now{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Outputs - note all in *local* time
  Output<Number> year;   // 2021-
  Output<Number> month;  // 1-12
  Output<Number> day;    // 1-31
  Output<Number> hour;   // 0-23
  Output<Number> minute; // 0-59
  Output<Number> second; // 0-59.999
  Output<Number> time;   // decimal HHMM: 0000-2359
};

//--------------------------------------------------------------------------
// Tick
void Now::tick(const TickData& td)
{
  const auto sample_rate = max(year.get_sample_rate(),
                               max(month.get_sample_rate(),
                                   max(day.get_sample_rate(),
                                       max(hour.get_sample_rate(),
                                           max(minute.get_sample_rate(),
                                               max(second.get_sample_rate(),
                                                   time.get_sample_rate()))))));
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, {},
                 tie(year, month, day, hour, minute, second, time),
                 [&](Number& year, Number& month, Number& day, Number& hour,
                     Number& minute, Number& second, Number& time)
  {
    Time::Stamp now = Time::Stamp::now().localise();
    Time::Split split = now.split();
    year   = split.year;
    month  = split.month;
    day    = split.day;
    hour   = split.hour;
    minute = split.min;
    second = split.sec;
    time   = split.hour*100 + split.min;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "now",
  "Now",
  "time",
  {},
  {},
  {
    { "year",    &Now::year   },
    { "month",   &Now::month  },
    { "day",     &Now::day    },
    { "hour",    &Now::hour   },
    { "minute",  &Now::minute },
    { "second",  &Now::second },
    { "time",    &Now::time   }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Now, module)
