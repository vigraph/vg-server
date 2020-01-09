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
  queue<Number> sample_buffer;
  Number total = 0.0;

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  AverageFilter *create_clone() const override
  {
    return new AverageFilter{module};
  }

  // Setup
  void setup(const SetupContext& context) override;

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Setting<bool> rms{false};
  Setting<Integer> samples{100};
  Input<Number> input{0.0};
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Sample
void AverageFilter::setup(const SetupContext& context)
{
  SimpleElement::setup(context);
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
  sample_iterate(td, nsamples, {}, tie(input), tie(output),
                 [&](Number input, Number& output)
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
