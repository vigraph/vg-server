//==========================================================================
// ViGraph dataflow module: audio/level/level.cc
//
// Audio level module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../audio-module.h"

namespace {

//==========================================================================
// Level filter
class LevelFilter: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  LevelFilter *create_clone() const override
  {
    return new LevelFilter{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<AudioData> input{0.0};
  Input<Number> gain{1.0};
  Output<AudioData> output;
};

//--------------------------------------------------------------------------
// Generate a fragment
void LevelFilter::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input, gain), tie(output),
                 [&](const AudioData& input, Number gain,
                     AudioData& o)
  {
    o.nchannels = input.nchannels;
    for(auto c=0; c<input.nchannels; c++)
      o.channels[c] = input.channels[c] * gain;
  });
}

Dataflow::SimpleModule module
{
  "level",
  "Level",
  "audio",
  {},
  {
    { "input", &LevelFilter::input },
    { "gain",  &LevelFilter::gain  }
  },
  {
    { "output", &LevelFilter::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(LevelFilter, module)
