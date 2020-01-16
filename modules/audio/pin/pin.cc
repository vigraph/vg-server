//==========================================================================
// ViGraph dataflow module: audio/pin/pin.cc
//
// Audio valued pin module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../audio-module.h"

namespace {

class AudioPin: public Pin<AudioData>
{
private:
  // Clone
  AudioPin *create_clone() const override
  {
    return new AudioPin{module};
  }
public:
  using Pin::Pin;
};

Dataflow::SimpleModule module
{
  "pin",
  "Audio pin",
  "audio",
  {},
  {
    { "input",  &AudioPin::input },
  },
  {
    { "output", &AudioPin::output },
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AudioPin, module)
