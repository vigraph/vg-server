//==========================================================================
// ViGraph vector graphics: vg-midi.h
//
// MIDI format reader/writer
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VG_MIDI_H
#define __VG_MIDI_H

#include <vector>
#include <deque>
#include <cstdint>

namespace ViGraph { namespace MIDI {

// Make our lives easier without polluting anyone else
using namespace std;
using namespace ViGraph;

//==========================================================================
// MIDI event
struct Event
{
  enum class Type
  {
    none,
    note_on,
    note_off,
    control_change
  };

  Type type{Type::none};
  uint8_t channel{0}; // Note, from 1 when valid
  uint8_t key{0};     // Key number or controller number (from 0)
  uint16_t value{0};  // velocity or controller value

  // Constructor
  Event() {}
  Event(Type _type, uint8_t _channel, uint8_t _key, uint16_t _value):
    type(_type), channel(_channel), key(_key), value(_value) {}
};

//==========================================================================
// MIDI format reader
class Reader
{
  uint8_t last_status{0};
  vector<uint8_t> data;
  deque<Event> events;

 public:
  //-----------------------------------------------------------------------
  // Constructor
  Reader() {}

  //-----------------------------------------------------------------------
  // Post a raw MIDI byte into the reader
  void add(uint8_t byte);

  //-----------------------------------------------------------------------
  // Get an event from the queue
  // Returns Event::Type::none if nothing available (yet)
  Event get();
};

//==========================================================================
}} //namespaces
#endif // !__VG_MIDI_H
