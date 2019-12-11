//==========================================================================
// ViGraph dataflow module: time-series/scale/scale.cc
//
// Scale time series values
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../time-series-module.h"

namespace {

//==========================================================================
// Scale filter
class ScaleFilter: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  ScaleFilter *create_clone() const override
  {
    return new ScaleFilter{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<DataCollection> input;
  Input<Number> factor{1.0};
  Output<DataCollection> output;
};

//--------------------------------------------------------------------------
// Tick data
void ScaleFilter::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input, factor), tie(output),
                 [&](const DataCollection& input, double factor,
                     DataCollection& output)
  {
    output = input;
    for(auto& ds: output.datasets)
      for(auto& s: ds.samples)
        s.value *= factor;
  });
}

Dataflow::SimpleModule module
{
  "scale",
  "Scale",
  "time-series",
  {},
  {
    { "input",  &ScaleFilter::input  },
    { "factor", &ScaleFilter::factor }
  },
  {
    { "output", &ScaleFilter::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ScaleFilter, module)
