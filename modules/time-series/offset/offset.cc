//==========================================================================
// ViGraph dataflow module: core/offset/offset.cc
//
// Add an offset to time series values
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../time-series-module.h"

namespace {

//==========================================================================
// Offset filter
class OffsetFilter: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  OffsetFilter *create_clone() const override
  {
    return new OffsetFilter{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<DataSet> input;
  Input<double> amount{0.0};
  Output<DataSet> output;
};

//--------------------------------------------------------------------------
// Tick data
void OffsetFilter::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(nsamples, {}, tie(input, amount), tie(output),
                 [&](const DataSet& input, double amount,
                     DataSet& output)
  {
    output = input;
    for(auto& s: output.samples)
      s.value += amount;
  });
}

Dataflow::SimpleModule module
{
  "offset",
  "Offset",
  "time-series",
  {},
  {
    { "input",  &OffsetFilter::input  },
    { "amount", &OffsetFilter::amount }
  },
  {
    { "output", &OffsetFilter::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(OffsetFilter, module)
