//==========================================================================
// ViGraph dataflow module: midi/pin/pin.cc
//
// MIDI pin module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../midi-module.h"

namespace {

class MIDIPin: public Pin<MIDIEvents>
{
private:
  // Clone
  MIDIPin *create_clone() const override
  {
    return new MIDIPin{module};
  }
public:
  using Pin::Pin;
};

Dataflow::SimpleModule module
{
  "pin",
  "MIDI Pin",
  "midi",
  {},
  {
    { "input",  &MIDIPin::input },
  },
  {
    { "output", &MIDIPin::output },
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MIDIPin, module)
