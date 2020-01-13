//==========================================================================
// ViGraph dataflow module: audio/bit-crush/bitcrush.cc
//
// BitCrush module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../audio-module.h"
#include "vg-geometry.h"

namespace {

const auto default_rate = 0;
const auto default_bits = 0;

using namespace ViGraph::Geometry;

//==========================================================================
// BitCrush
class BitCrush: public SimpleElement
{
private:
  AudioData last_sample;
  unsigned samples_ago = numeric_limits<unsigned>::max() - 1;

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  BitCrush *create_clone() const override
  {
    return new BitCrush{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<AudioData> input;
  Input<Number> rate{default_rate};
  Input<Number> bits{default_bits};
  Output<AudioData> output;
};

//--------------------------------------------------------------------------
// Tick data
void BitCrush::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, tie(input, rate, bits), tie(output),
                 [&](const AudioData& i, Number r, Number b, AudioData& o)
  {
    if (!r || ++samples_ago >= sample_rate / r)
    {
      last_sample = o = i;
      samples_ago = 0;
    }
    else
    {
      o = last_sample;
    }
    b = round(b);
    if (b)
    {
      const auto max = pow(2, b) - 1;
      for(auto c=0; c<o.nchannels; c++)
        o.channels[c] = (2.0 * (round(((o.channels[c] + 1.0) * 0.5) * max)
                                / max)) - 1.0;
    }
  });
}

Dataflow::SimpleModule module
{
  "bit-crush",
  "Bit Crush",
  "audio",
  {},
  {
    { "input",    &BitCrush::input },
    { "rate",     &BitCrush::rate },
    { "bits",     &BitCrush::bits },
  },
  {
    { "output",   &BitCrush::output },
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(BitCrush, module)
