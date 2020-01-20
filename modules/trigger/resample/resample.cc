//==========================================================================
// Resamples a trigger stream
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"
#include <cmath>
#include <stdlib.h>

namespace {

//==========================================================================
// Resample
class Resample: public SimpleElement
{
 private:
  // Element virtuals
  void tick(const TickData& td) override;
  void setup(const SetupContext& context) override;

  // We control our own input sample rate
  void update_sample_rate() override {}

  // Clone
  Resample *create_clone() const override
  {
    return new Resample{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Setting<Number> input_rate{1000.0};

  // Inputs
  Input<Trigger> input{0.0};

  // Outputs
  Output<Trigger> output;
};

//--------------------------------------------------------------------------
// Setup
void Resample::setup(const SetupContext& context)
{
  SimpleElement::setup(context);

  input.set_sample_rate(input_rate);
}

//--------------------------------------------------------------------------
// Tick
void Resample::tick(const TickData& td)
{
  const auto output_rate = output.get_sample_rate();
  const auto nsamples = td.samples_in_tick(output_rate);

  const auto& ibuff = input.get_buffer();
  const auto obuff = output.get_buffer(td);

  if (nsamples)
  {
    const auto rate = static_cast<double>(ibuff.size()) / nsamples;
    obuff.data.resize(nsamples);
    auto i = 0u;
    for (const auto& in: ibuff)
    {
      obuff.data[i / rate] = in;
      ++i;
    }
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "resample",
  "Resample",
  "trigger",
  {
    { "input-rate", &Resample::input_rate },
  },
  {
    { "input",      &Resample::input },
  },
  {
    { "output",     &Resample::output },
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Resample, module)
