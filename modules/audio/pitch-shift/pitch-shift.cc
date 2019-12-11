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
  Input<Number> input{0.0};
  Input<Number> pitch{0.0};
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Constructor
PitchShift::PitchShift(const SimpleModule& module):
  SimpleElement{module}
{
#if defined(PLATFORM_WINDOWS)
  soundtouch.reset(soundtouch_createInstance());
  soundtouch_setChannels(sound_touch.get(), 1);
#else
  sound_touch.setChannels(1);
#endif

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
  f.reserve(nsamples);
  sample_iterate(td, nsamples, {}, tie(input), {},
                 [&](double input)
  {
    f.emplace_back(input);
  });

#if defined(PLATFORM_WINDOWS)
  soundtouch_setPitchSemiTones(sound_touch.get(), pitch);
  if (!f.empty())
    soundtouch_putSamples(sound_touch.get(), &f[0], f.size());
  const auto max_samples = min(soundtouch_numSamples(sound_touch.get()),
                               static_cast<unsigned>(nsamples));
#else
  sound_touch.setPitchSemiTones(pitch);
  if (!f.empty())
    sound_touch.putSamples(&f[0], f.size());
  const auto max_samples = min(sound_touch.numSamples(),
                               static_cast<unsigned>(nsamples));
#endif

  f.clear();
  if (max_samples)
  {
    f.resize(max_samples);
#if defined(PLATFORM_WINDOWS)
    const auto samples = soundtouch_receiveSamples(sound_touch.get(),
                                                   &f[0], f.size());
#else
    const auto samples = sound_touch.receiveSamples(&f[0], f.size());
#endif
    f.resize(samples);
  }

  auto fpos = 0u;
  sample_iterate(td, nsamples, {}, {}, tie(output),
                 [&](double& o)
  {
    if (fpos < f.size())
      o = f[fpos++];
    else
      o = 0;
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
