//==========================================================================
// Waveform valued pin module
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../waveform-module.h"

namespace {

class WaveformPin: public Pin<Waveform::Type>
{
private:
  // Clone
  WaveformPin *create_clone() const override
  {
    return new WaveformPin{module};
  }
public:
  using Pin::Pin;
};

Dataflow::SimpleModule module
{
  "pin",
  "Waveform pin",
  "waveform",
  {},
  {
    { "input",  &WaveformPin::input },
  },
  {
    { "output", &WaveformPin::output },
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(WaveformPin, module)
