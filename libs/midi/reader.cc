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
// Post a raw MIDI byte into the reader
void Reader::add(uint8_t byte)
{
  // Status or data?
  if (byte & 0x80)
  {
    // Ignore realtime messages (for now!)
    if (byte < 0xF8)
    {
      // Keep it for later - including for 'running status'
      last_status = byte;
      data.clear();
    }
  }
  else
  {
    data.push_back(byte);
    if (last_status)
    {
      // Check for enough data now
      const auto cmd = (last_status >> 4) & 7; // Ignore known top bit
      const auto chan = (last_status & 0xf) + 1;

      switch (cmd)
      {
        case 0:  // Note off
          if (data.size() >= 2)
          {
            data.clear();
            events.push_back(Event(Event::Type::note_off, chan,
                                   data[0], data[1]));
          }
        break;

        case 1:  // Note on
          if (data.size() >= 2)
          {
            data.clear();
            events.push_back(Event(Event::Type::note_on, chan,
                                   data[0], data[1]));
          }
        break;

        case 3:  // Control change
          if (data.size() >= 2)
          {
            data.clear();
            events.push_back(Event(Event::Type::control_change, chan,
                                   data[0], data[1]));
          }
        break;

      }
    }
  }
}

//-----------------------------------------------------------------------
// Get an event from the buffer
// Returns Event::Type::none if nothing available (yet)
Event Reader::get()
{
  if (events.empty())
    return Event();
  else
  {
    const auto event = events.front();
    events.pop_front();
    return event;
  }
}

}} // namespaces
