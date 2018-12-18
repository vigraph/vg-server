//==========================================================================
// ViGraph MIDI library: reader.cc
//
// MIDI format reader
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-midi.h"

namespace ViGraph { namespace MIDI {

//-----------------------------------------------------------------------
// Read an event from the buffer
// Returns Event::Type::none if nothing available (yet)
Event Reader::read()
{
  // If top bit not set when we enter here, we're out of sync or in a
  // system exclusive message or something else we don't handle
  // - either way, just drop them
  while (!buffer.empty() && !(buffer[0] & 0x80))
    buffer.erase(buffer.begin());

  if (!buffer.empty())
  {
    const auto first = buffer[0];
    const auto cmd = (first >> 4) & 7; // We know top bit is set, ignore it
    const auto chan = (first & 0xf) + 1;

    switch (cmd)
    {
      case 0:  // Note off
        if (buffer.size() >= 3)
        {
          buffer.erase(buffer.begin(), buffer.begin()+3);
          return Event(Event::Type::note_off, chan, buffer[1], buffer[2]);
        }
      break;

      case 1:  // Note on
        if (buffer.size() >= 3)
        {
          buffer.erase(buffer.begin(), buffer.begin()+3);
          return Event(Event::Type::note_on, chan, buffer[1], buffer[2]);
        }
      break;
    }
  }

  return Event();
}

}} // namespaces
