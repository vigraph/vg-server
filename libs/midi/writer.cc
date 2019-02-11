//==========================================================================
// ViGraph MIDI library: writer.cc
//
// MIDI format writer
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-midi.h"

namespace ViGraph { namespace MIDI {

//-----------------------------------------------------------------------
// Write and event to the buffer
void Writer::write(const Event& event)
{
  switch (event.type)
  {
    case Event::Type::none:
      return;
    case Event::Type::note_on:
      data.push_back(0x90 + ((event.channel - 1) & 0x0f));
      break;
    case Event::Type::note_off:
      data.push_back(0x80 + ((event.channel - 1) & 0x0f));
      break;
    case Event::Type::control_change:
      data.push_back(0xB0 + ((event.channel - 1) & 0x0f));
      break;
  }
  data.push_back(event.key);
  data.push_back(event.value);
}

}} // namespaces
