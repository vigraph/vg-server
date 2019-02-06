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
  SoundTouch sound_touch;

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
  sound_touch.setSampleRate(sample_rate);
  sound_touch.setPitchSemiTones(config.get_attr_real("pitch", 0));
}

//--------------------------------------------------------------------------
// Set a control property
void ShiftFilter::set_property(const string& property, const SetParams& sp)
{
  if (property == "pitch")
    sound_touch.setPitchSemiTones(sp.v.d);
}

//--------------------------------------------------------------------------
// Process some data
void ShiftFilter::accept(FragmentPtr fragment)
{
  if (fragment->nchannels != sound_touch.numChannels())
  {
    sound_touch.flush();
    sound_touch.setChannels(fragment->nchannels);
  }

  sound_touch.putSamples(&fragment->waveform[0], fragment->waveform.size()/
                                                 fragment->nchannels);
  const auto samples = sound_touch.receiveSamples(&fragment->waveform[0],
                                                  fragment->waveform.size()
                                                  / fragment->nchannels);
  if (samples)
  {
    fragment->waveform.resize(samples * fragment->nchannels);
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
