//==========================================================================
// ViGraph midi modules: midi-module.h
//
// Common definitions for midi modules
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_MIDI_MODULE_H
#define __VIGRAPH_MIDI_MODULE_H

#include "../module.h"
#include "vg-dataflow.h"
#include "vg-midi.h"
#include <algorithm>

namespace ViGraph { namespace Dataflow {

struct MIDIEvent: public MIDI::Event
{
  Time::Duration time;

  // Constructor
  MIDIEvent() {}
  MIDIEvent(const Time::Duration& _time, const MIDI::Event& event):
    MIDI::Event{event}, time{_time}
  {}
  MIDIEvent(const Time::Duration& _time, Type type, uint8_t channel,
            uint8_t key, uint16_t value):
    MIDI::Event{type, channel, key, value}, time{_time}
  {}

  // Comparison operator
  bool operator<(const MIDIEvent& b) const
  {
    return time < b.time;
  }
};

using MIDIEvents = vector<MIDIEvent>;

// Combine
MIDIEvents& operator+=(MIDIEvents& a, const MIDIEvents& b)
{
  a.insert(a.end(), b.begin(), b.end());
  return a;
}

// Downsample
template<>
inline void downsample(const vector<MIDIEvents>& from, vector<MIDIEvents>& to)
{
  const auto fsize = from.size();
  const auto tsize = to.size();
  for (auto i = 0u; i < tsize; ++i)
  {
    auto t = MIDIEvents{};
    const auto b = (i * fsize) / tsize;
    const auto e = ((i + 1) * fsize) / tsize;
    for (auto j = b; j < e; ++j)
      t.insert(t.end(), from[j].begin(), from[j].end());
    to[i] = t;
  }
}

template<> inline
string get_module_type<MIDIEvents>() { return "midi"; }

template<> inline void set_from_json(MIDIEvents& events,
                                     const JSON::Value& json)
{
  if (json.type != JSON::Value::Type::ARRAY)
    return;

  for (const auto& json_e: json.a)
  {
    auto event = MIDIEvent{};
    event.time = json_e["time"].as_str();
    const auto t = json_e["type"].as_str();
    if (t == "note-on")
      event.type = MIDI::Event::Type::note_on;
    else if (t == "note-off")
      event.type = MIDI::Event::Type::note_off;
    else if (t == "control-change")
      event.type = MIDI::Event::Type::control_change;
    else
      event.type = MIDI::Event::Type::none;
    switch (event.type)
    {
      case MIDI::Event::Type::note_on:
      case MIDI::Event::Type::note_off:
        event.channel = json_e["channel"].as_int();
        event.key = json_e["key"].as_int();
        event.value = json_e["velocity"].as_int();
        break;
      case MIDI::Event::Type::control_change:
        event.channel = json_e["channel"].as_int();
        event.key = json_e["control"].as_int();
        event.value = json_e["value"].as_int();
        break;
      case MIDI::Event::Type::none:
        break;
    }
    events.emplace_back(event);
  }
}

template<> inline JSON::Value get_as_json(const MIDIEvents& events)
{
  JSON::Value json{JSON::Value::Type::ARRAY};
  for (const auto& event: events)
  {
    auto& json_e = json.add(JSON::Value::Type::OBJECT);
    json_e.put("time", event.time.iso());
    switch (event.type)
    {
      case MIDI::Event::Type::note_on:
      case MIDI::Event::Type::note_off:
        json_e.put("type", event.type == MIDI::Event::Type::note_on
                           ? "note-on" : "note-off");
        json_e.put("channel", event.channel);
        json_e.put("key", event.key);
        json_e.put("velocity", event.value);
        break;
      case MIDI::Event::Type::control_change:
        json_e.put("type", "control-change");
        json_e.put("channel", event.channel);
        json_e.put("control", event.key);
        json_e.put("value", event.value);
        break;
      case MIDI::Event::Type::none:
        json_e.put("type", "none");
        break;
    }
  }
  return json;
}

//==========================================================================
}} //namespaces

#endif // !__VIGRAPH_MIDI_MODULE_H
