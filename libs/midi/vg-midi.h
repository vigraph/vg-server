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
#include <string>

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
  uint8_t voice{0};

  // Constructor
  Event() {}
  Event(Type _type, uint8_t _channel,
        uint8_t _key, uint16_t _value):
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
// MIDI format writer
class Writer
{
private:
  vector<uint8_t>& data;

public:
  //-----------------------------------------------------------------------
  // Constructor
  Writer(vector<uint8_t>& _data): data{_data} {}

  //-----------------------------------------------------------------------
  // Write a MIDI event to the buffer
  void write(const Event &event);
};

//==========================================================================
// MIDI Number from note
// Returns midi note number or -1 on error
int get_midi_number(const string& note);

//==========================================================================
// MIDI Note from number
// Returns midi note or empty string on error
string get_midi_note(int number);

//==========================================================================
// MIDI Note frequency
double get_midi_frequency(int number);

//==========================================================================
// MIDI Number from frequency
int get_midi_frequency_number(double frequency);

//==========================================================================
}} //namespaces
#endif // !__VG_MIDI_H
