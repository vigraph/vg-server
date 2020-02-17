//==========================================================================
// ViGraph dataflow module: audio/pitch-shift/pitch-shift.cc
//
// Pitch shifter module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../audio-module.h"
#if defined(PLATFORM_WINDOWS)
#include "SoundTouchDLL.h"
#else
#include "SoundTouch.h"
#endif

namespace {

#if !defined(PLATFORM_WINDOWS)
using namespace soundtouch;
#endif

//==========================================================================
// Pitch Shift
class PitchShift: public SimpleElement
{
private:
#if defined(PLATFORM_WINDOWS)
  unique_ptr<remove_pointer<HANDLE>::type,
             decltype(&soundtouch_destroyInstance)> sound_touch;
#else
  SoundTouch sound_touch;
#endif

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  PitchShift *create_clone() const override
  {
    return new PitchShift{module};
  }

public:
  PitchShift(const SimpleModule& module);

  // Configuration
  Input<AudioData> input{0.0};
  Input<Number> pitch{0.0};
  Output<AudioData> output;
};

//--------------------------------------------------------------------------
// Constructor
PitchShift::PitchShift(const SimpleModule& module):
  SimpleElement{module}
#if defined(PLATFORM_WINDOWS)
  , sound_touch(soundtouch_createInstance(), soundtouch_destroyInstance)
#endif
{
}

//--------------------------------------------------------------------------
// Tick data
void PitchShift::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();
#if defined(PLATFORM_WINDOWS)
  soundtouch_setSampleRate(sound_touch.get(), sample_rate);
#else
  sound_touch.setSampleRate(sample_rate);
#endif

  const auto nsamples = td.samples_in_tick(sample_rate);

  auto f = vector<float>{};
  auto nchannels = 0u;
  sample_iterate(td, nsamples, {}, tie(input), {},
                 [&](const AudioData& input)
  {
    if (!nchannels)
    {
      nchannels = input.nchannels;
      f.reserve(nsamples * nchannels);
#if defined(PLATFORM_WINDOWS)
      soundtouch_setChannels(sound_touch.get(), nchannels);
#else
      sound_touch.setChannels(nchannels);
#endif
    }
    for (auto c = 0u; c < nchannels; ++c)
      f.emplace_back(input.channels[c]);
  });

#if defined(PLATFORM_WINDOWS)
  soundtouch_setPitchSemiTones(sound_touch.get(), pitch);
  if (!f.empty())
    soundtouch_putSamples(sound_touch.get(), &f[0], f.size() / nchannels);
  const auto available_samples = soundtouch_numSamples(sound_touch.get())
                                 / nchannels;
#else
  sound_touch.setPitchSemiTones(pitch);
  if (!f.empty())
    sound_touch.putSamples(&f[0], f.size() / nchannels);
  const auto available_samples = sound_touch.numSamples() / nchannels;
#endif
  const auto max_samples = nchannels
                           ? min(available_samples,
                                 static_cast<unsigned>(nsamples))
                           : 0;

  f.clear();
  if (max_samples)
  {
    f.resize(max_samples * nchannels);
#if defined(PLATFORM_WINDOWS)
    const auto samples = soundtouch_receiveSamples(sound_touch.get(),
                                                   &f[0], max_samples);
#else
    const auto samples = sound_touch.receiveSamples(&f[0], max_samples);
#endif
    f.resize(samples * nchannels);
  }

  auto fpos = 0u;
  sample_iterate(td, nsamples, {}, {}, tie(output),
                 [&](AudioData& o)
  {
    o = AudioData();
    o.nchannels = nchannels;
    if (fpos < f.size())
    {
      for (auto c = 0u; c < nchannels; ++c)
        o.channels[c] = f[fpos++];
    }
  });
}

Dataflow::SimpleModule module
{
  "pitch-shift",
  "Pitch Shift",
  "audio",
  {},
  {
    { "input",  &PitchShift::input },
    { "pitch",  &PitchShift::pitch }
  },
  {
    { "output", &PitchShift::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(PitchShift, module)
