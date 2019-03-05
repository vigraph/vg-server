//==========================================================================
// ViGraph dataflow module:
//    audio/filters/shift/shift.cc
//
// Audio shift filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"
#include "SoundTouch.h"

namespace {

using namespace ViGraph::Dataflow;
using namespace soundtouch;

//==========================================================================
// Shift filter
class ShiftFilter: public FragmentFilter
{
  map<Speaker, SoundTouch> sound_touch;
  double pitch = 0;

  // Source/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FragmentPtr fragment) override;

public:
  ShiftFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <shift pitch="0.125" />
ShiftFilter::ShiftFilter(const Dataflow::Module *module,
                         const XML::Element& config):
    Element(module, config), FragmentFilter(module, config)
{
  pitch = config.get_attr_real("pitch", pitch);
}

//--------------------------------------------------------------------------
// Set a control property
void ShiftFilter::set_property(const string& property, const SetParams& sp)
{
  if (property == "pitch")
  {
    pitch = sp.v.d;
    for (auto &st: sound_touch)
      st.second.setPitchSemiTones(pitch);
  }
}

//--------------------------------------------------------------------------
// Process some data
void ShiftFilter::accept(FragmentPtr fragment)
{
  auto max_samples = 0xffffffffffffffff;
  for (auto& wit: fragment->waveforms)
  {
    const auto c = wit.first;
    auto& w = wit.second;

    auto stit = sound_touch.find(c);
    if (stit == sound_touch.end())
    {
      // SoundTouch doesn't copy well, so this defers construction until
      // it's in place
      stit = sound_touch.emplace(piecewise_construct,
                                 forward_as_tuple(c),
                                 forward_as_tuple()).first;
      stit->second.setChannels(1);
      stit->second.setSampleRate(sample_rate);
      stit->second.setPitchSemiTones(pitch);
    }

    stit->second.putSamples(&w[0], w.size());
    max_samples = min(max_samples,
                      min(w.size(), static_cast<unsigned long>(
                                    stit->second.numSamples())));
  }

  if (max_samples)
  {
    for (auto& wit: fragment->waveforms)
    {
      const auto c = wit.first;
      auto& w = wit.second;
      auto stit = sound_touch.find(c);
      const auto samples = stit->second.receiveSamples(&w[0], max_samples);
      w.resize(samples);
    }

    send(fragment);
  }
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
    { "pitch", { {"Pitch semi tones (-60 to +60", "0"}, Value::Type::number,
                                                        "@pitch", true } },
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ShiftFilter, module)
