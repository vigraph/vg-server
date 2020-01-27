//==========================================================================
// MIDI Switch module
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../midi-module.h"
#include "../../switch.h"

namespace {

class MIDISwitch: public Switch<MIDIEvents>
{
public:
  const static Dataflow::DynamicModule switch_module;

private:
  // Clone
  MIDISwitch *create_clone() const override
  {
    return new MIDISwitch{switch_module};
  }
public:
  using Switch::Switch;
};

const Dataflow::DynamicModule MIDISwitch::switch_module =
{
  "switch",
  "MIDI Switch",
  "midi",
  {
    { "inputs",         &MIDISwitch::inputs },
  },
  {
    { "number",         &MIDISwitch::number },
    { "fraction",       &MIDISwitch::fraction },
    { "next",           &MIDISwitch::next },
  },
  {
    { "output",         &MIDISwitch::output },
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MIDISwitch, MIDISwitch::switch_module)
