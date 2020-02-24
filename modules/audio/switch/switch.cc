//==========================================================================
// ViGraph dataflow module: audio/switch/switch.cc
//
// Switch module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../audio-module.h"
#include "../../switch.h"

template<>
inline AudioData switch_fade(const AudioData& value, Number factor)
{
  AudioData result;
  result.nchannels = value.nchannels;
  for(auto c=0; c<result.nchannels; c++)
    result.channels[c] = value.channels[c] * factor;
  return result;
}

namespace {

class AudioSwitch: public FadeableSwitch<AudioData>
{
public:
  const static Dataflow::DynamicModule switch_module;

private:
  // Clone
  AudioSwitch *create_clone() const override
  {
    return new AudioSwitch{switch_module};
  }
public:
  using FadeableSwitch::FadeableSwitch;
};

const Dataflow::DynamicModule AudioSwitch::switch_module =
{
  "switch",
  "Switch",
  "audio",
  {
    { "inputs",         &AudioSwitch::inputs },
  },
  {
    { "number",         &AudioSwitch::number },
    { "fraction",       &AudioSwitch::fraction },
    { "next",           &AudioSwitch::next },
    { "fade-in-time",   &AudioSwitch::fade_in_time },
    { "fade-out-time",  &AudioSwitch::fade_out_time },
  },
  {
    { "output",         &AudioSwitch::output },
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AudioSwitch, AudioSwitch::switch_module)
