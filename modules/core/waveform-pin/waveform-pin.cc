//==========================================================================
// ViGraph dataflow module: core/waveform-pin/waveform-pin.cc
//
// Waveform valued pin module
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../core-module.h"
#include "../../waveform.h"

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
  "waveform-pin",
  "Waveform pin",
  "core",
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
