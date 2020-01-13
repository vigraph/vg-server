//==========================================================================
// ViGraph audio modules: audio-module.h
//
// Common definitions for audio modules
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_AUDIO_MODULE_H
#define __VIGRAPH_AUDIO_MODULE_H

#include "../module.h"

namespace ViGraph { namespace Module { namespace Audio {

// We follow Audacity in choosing 32-bit float and 44100 sample rate -
// high resolution without creating unnecessary load and file size
// - plus easy to read to/from WAV, CD and audio IO
using sample_t = float;           // Unsigned linear PCM -1.0..1.0
const unsigned int sample_rate = 44100;

}}};

using namespace ViGraph::Module::Audio;

namespace ViGraph { namespace Dataflow {

//--------------------------------------------------------------------------
// Audio data
struct AudioData
{
  static const size_t max_channels = 8;
  int nchannels{0};                         // Number of samples valid in...
  array<sample_t, max_channels> channels;

  AudioData() {}
  AudioData(const decltype(channels)& _c):
    nchannels(_c.size()), channels(_c) {}
  // Special for tests, single item
  AudioData(sample_t s): nchannels(1) { channels[0] = s; }

  // Combine operator
  AudioData& operator+=(const AudioData& o)
  {
    auto same_channels = min(nchannels, o.nchannels);
    for(auto c=0; c<same_channels; c++)  // Both have
      channels[c] += o.channels[c];
    for(auto c=same_channels; c<o.nchannels; c++)  // Only they have
      channels[c] = o.channels[c];
    // Ones only we have can be left alone

    nchannels = max(nchannels, o.nchannels);
    return *this;
  }
};

template<> inline
string get_module_type<AudioData>() { return "audio"; }

template<> inline void set_from_json(AudioData& audio,
                                     const JSON::Value& json)
{
  if (json.type == JSON::Value::OBJECT)
  {
    audio.nchannels = json["n"].as_int();
    const auto& jchans = json["c"];
    if (jchans.type == JSON::Value::ARRAY)
    {
      for(auto i=0; i<audio.nchannels; i++)
        audio.channels[i] = jchans[i].as_float();
    }
  }
}

template<> inline JSON::Value get_as_json(const AudioData& audio)
{
  JSON::Value value{JSON::Value::OBJECT};
  value["n"] = audio.nchannels;
  auto& jchans = value.put("c", JSON::Value::ARRAY);
  for(auto i=0; i<audio.nchannels; i++)
    jchans.add(audio.channels[i]);

  return value;
}

//==========================================================================
}} //namespaces

using namespace ViGraph::Module::Audio;

#endif // !__VIGRAPH_AUDIO_MODULE_H
