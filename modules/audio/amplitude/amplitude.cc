//==========================================================================
// ViGraph dataflow module: audio/amplitude/amplitude.cc
//
// Audio amplitude module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../audio-module.h"

namespace {

//==========================================================================
// Amplitude filter
class AmplitudeFilter: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  AmplitudeFilter *create_clone() const override
  {
    return new AmplitudeFilter{module};
  }

public:
  using SimpleElement::SimpleElement;

  Setting<Integer> channel{0};
  Input<AudioData> input{0.0};
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Generate a fragment
void AmplitudeFilter::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input), tie(output),
                 [&](const AudioData& input, Number& output)
  {
    if (channel < input.nchannels)
      output = input.channels[channel] / 2.0 + 0.5;  // Range to 0..1
    else
      output = 0;
  });
}

Dataflow::SimpleModule module
{
  "amplitude",
  "Amplitude",
  "audio",
  {
    { "channel", &AmplitudeFilter::channel }
  },
  {
    { "input",   &AmplitudeFilter::input }
  },
  {
    { "output", &AmplitudeFilter::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AmplitudeFilter, module)
