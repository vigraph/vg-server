//==========================================================================
// ViGraph dataflow module: time-series/average/average.cc
//
// Box-car rolling average for time series values
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../time-series-module.h"

namespace {

//==========================================================================
// Average filter
class AverageFilter: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  AverageFilter *create_clone() const override
  {
    return new AverageFilter{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<Number> samples{0.0};

  // Input
  Input<DataCollection> input;

  // Output
  Output<DataCollection> output;
};

//--------------------------------------------------------------------------
// Tick data
void AverageFilter::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input, samples), tie(output),
                 [&](const DataCollection& input, double samples,
                     DataCollection& output)
  {
    // Special case
    if (!samples)
    {
      output = input;
      return;
    }

    for(auto& ids: input.datasets)
    {
      DataSet ods;
      ods.name = ids.name;
      ods.source = ids.source + ", average(" + Text::itos(samples) + ")";
      auto total{0.0};

      for(auto i=0u; i<ids.samples.size(); i++)
      {
        const auto& s = ids.samples[i];

        // Generate previous total if enough
        if (i >= samples)
        {
          // Place the average at the point midway in this span
          ods.add(ids.samples[i-samples/2].at, total/samples);

          // Remove the last one
          total -= ids.samples[i-samples].value;
        }

        // Rolling total
        total += s.value;
      }

      output.add(ods);
    }
  });
}

Dataflow::SimpleModule module
{
  "average",
  "Average",
  "time-series",
  {},
  {
    { "input",   &AverageFilter::input   },
    { "samples", &AverageFilter::samples }
  },
  {
    { "output", &AverageFilter::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AverageFilter, module)
