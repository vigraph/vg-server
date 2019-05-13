//==========================================================================
// ViGraph dataflow module:
//    audio/filters/shift/shift.cc
//
// Audio shift filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"
#if defined(PLATFORM_WINDOWS)
#include "SoundTouchDLL.h"
#else
#include "SoundTouch.h"
#endif

namespace {

using namespace ViGraph::Dataflow;
#if !defined(PLATFORM_WINDOWS)
using namespace soundtouch;
#endif

//==========================================================================
// Shift filter
class ShiftFilter: public FragmentFilter
{
#if defined(PLATFORM_WINDOWS)
  map<Speaker, unique_ptr<remove_pointer<HANDLE>::type,
                          decltype(&soundtouch_destroyInstance)>> sound_touch;
#else
  map<Speaker, SoundTouch> sound_touch;
#endif
  double pitch = 0;
  bool enabled = true;

  // Source/Element virtuals
  void accept(FragmentPtr fragment) override;
  void notify_target_of(const string& property) override;

public:
  using FragmentFilter::FragmentFilter;

  // Getters/Setters
  double get_pitch() const { return pitch; }
  void set_pitch(double pitch);
  void on() { enabled = true; }
  void off() { enabled = false; sound_touch.clear(); }
};

//--------------------------------------------------------------------------
// Set pitch
void ShiftFilter::set_pitch(double p)
{
  pitch = p;
  for (auto &st: sound_touch)
  {
#if defined(PLATFORM_WINDOWS)
    soundtouch_setPitchSemiTones(st.second.get(), pitch);
#else
    st.second.setPitchSemiTones(pitch);
#endif
  }
}

//--------------------------------------------------------------------------
// Process some data
void ShiftFilter::accept(FragmentPtr fragment)
{
  if (enabled)
  {
    auto max_samples = 0xffffffffffffffff;
    for (auto& wit: fragment->waveforms)
    {
      const auto c = wit.first;
      auto& w = wit.second;

      auto stit = sound_touch.find(c);
      if (stit == sound_touch.end())
      {
#if defined(PLATFORM_WINDOWS)
        stit = sound_touch.emplace(piecewise_construct,
                                   forward_as_tuple(c),
                                   forward_as_tuple(
                                     soundtouch_createInstance(),
                                     soundtouch_destroyInstance)).first;
        soundtouch_setChannels(stit->second.get(), 1);
        soundtouch_setSampleRate(stit->second.get(), sample_rate);
        soundtouch_setPitchSemiTones(stit->second.get(), pitch);
#else
        // SoundTouch doesn't copy well, so this defers construction until
        // it's in place
        stit = sound_touch.emplace(piecewise_construct,
                                   forward_as_tuple(c),
                                   forward_as_tuple()).first;
        stit->second.setChannels(1);
        stit->second.setSampleRate(sample_rate);
        stit->second.setPitchSemiTones(pitch);
#endif
      }

#if defined(PLATFORM_WINDOWS)
      soundtouch_putSamples(stit->second.get(), &w[0], w.size());
      max_samples = min(max_samples,
                        min(w.size(), static_cast<vector<float>::size_type>(
                                soundtouch_numSamples(stit->second.get()))));
#else
      stit->second.putSamples(&w[0], w.size());
      max_samples = min(max_samples,
                        min(w.size(), static_cast<unsigned long>(
                                      stit->second.numSamples())));
#endif
    }

    if (max_samples)
    {
      for (auto& wit: fragment->waveforms)
      {
        const auto c = wit.first;
        auto& w = wit.second;
        auto stit = sound_touch.find(c);
#if defined(PLATFORM_WINDOWS)
        const auto samples = soundtouch_receiveSamples(stit->second.get(),
                                                       &w[0], max_samples);
#else
        const auto samples = stit->second.receiveSamples(&w[0], max_samples);
#endif
        w.resize(samples);
      }

      send(fragment);
    }
  }
  else
  {
    send(fragment);
  }
}

//--------------------------------------------------------------------------
// If recipient of triggers default to disabled
void ShiftFilter::notify_target_of(const string& property)
{
  if (property == "enable")
    enabled = false;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "shift",
  "Audio shift",
  "Audio shift (pitch)",
  "audio",
  {
    { "pitch", { "Pitch semi tones (-60 to +60", Value::Type::number,
                 { &ShiftFilter::get_pitch, &ShiftFilter::set_pitch}, true } },
    { "enable", { "Enable the filter", Value::Type::trigger,
                  &ShiftFilter::on, true } },
    { "disable", { "Disable the filter", Value::Type::trigger,
                   &ShiftFilter::off, true } },
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ShiftFilter, module)
