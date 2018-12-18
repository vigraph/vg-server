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
    note_off
  };

  Type type{Type::none};
  uint8_t channel{0}; // Note, from 1 when valid
  uint8_t key{0};
  uint8_t value{0};   // velocity or controller value

  // Constructor
  Event() {}
  Event(Type _type, uint8_t _channel, uint8_t _key, uint8_t _value):
    type(_type), channel(_channel), key(_key), value(_value) {}
};

//==========================================================================
// MIDI format reader
class Reader
{
  vector<uint8_t> buffer;

 public:
  //-----------------------------------------------------------------------
  // Constructor
  Reader() {}

  //-----------------------------------------------------------------------
  // Post a MIDI byte into the buffer
  void add(uint8_t byte) { buffer.push_back(byte); }

  //-----------------------------------------------------------------------
  // Read an event from the buffer
  // Returns Event::Type::none if nothing available (yet)
  Event read();
};

//==========================================================================
}} //namespaces
#endif // !__VG_MIDI_H
