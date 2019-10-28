//==========================================================================
// ViGraph dataflow module: core/average/average.cc
//
// Average module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../core-module.h"

namespace {

//==========================================================================
// Average filter
class AverageFilter: public SimpleElement
{
private:
  queue<double> sample_buffer;
  double total = 0.0;

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  AverageFilter *create_clone() const override
  {
    return new AverageFilter{module};
  }

  // Setup
  void setup() override;

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Setting<bool> rms{false};
  Setting<int> samples{100};
  Input<double> input{0.0};
  Output<double> output;
};

//--------------------------------------------------------------------------
// Sample
void AverageFilter::setup()
{
  if (samples < 1)
    samples = 1;
  while (sample_buffer.size() > static_cast<unsigned>(samples))
  {
    if (rms)
      total -= sample_buffer.front() * sample_buffer.front();
    else
      total -= sample_buffer.front();
    sample_buffer.pop();
  }
}

//--------------------------------------------------------------------------
// Tick data
void AverageFilter::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(nsamples, {}, tie(input), tie(output),
                 [&](double input, double& output)
  {
    sample_buffer.push(input);
    if (rms)
      total += input * input;
    else
      total += input;
    if (sample_buffer.size() > static_cast<unsigned>(samples))
    {
      if (rms)
        total -= sample_buffer.front() * sample_buffer.front();
      else
        total -= sample_buffer.front();
      sample_buffer.pop();
    }
    output = rms ? sqrt(total / sample_buffer.size())
                 : total / sample_buffer.size();
  });
}

Dataflow::SimpleModule module
{
  "average",
  "Average",
  "core",
  {
    { "rms",      &AverageFilter::rms },
    { "samples",  &AverageFilter::samples },
  },
  {
    { "input",  &AverageFilter::input },
  },
  {
    { "output", &AverageFilter::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AverageFilter, module)
