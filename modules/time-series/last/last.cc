//==========================================================================
// ViGraph dataflow module: time-series/last/last.cc
//
// Select last N samples for time series values
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../time-series-module.h"

namespace {

//==========================================================================
// Last filter
class LastFilter: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  LastFilter *create_clone() const override
  {
    return new LastFilter{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<double> samples{0.0};

  // Input
  Input<DataCollection> input;

  // Output
  Output<DataCollection> output;
};

//--------------------------------------------------------------------------
// Tick data
void LastFilter::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(nsamples, {}, tie(input, samples), tie(output),
                 [&](const DataCollection& input, double samples,
                     DataCollection& output)
  {
    for(auto& ids: input.datasets)
    {
      DataSet ods;
      ods.name = ids.name;
      ods.source = ids.source + ", last(" + Text::itos(samples) + ")";

      // Find start position, samples off end, or 0 if not enough
      auto start = 0u;
      if (ids.samples.size() > samples) start = ids.samples.size()-samples;

      for(auto i=start; i<ids.samples.size(); i++)
      {
        const auto& s = ids.samples[i];

        // Copy this one
        ods.add(s.at, s.value);
      }

      output.add(ods);
    }
  });
}

Dataflow::SimpleModule module
{
  "last",
  "Last",
  "time-series",
  {},
  {
    { "input",   &LastFilter::input   },
    { "samples", &LastFilter::samples }
  },
  {
    { "output", &LastFilter::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(LastFilter, module)
