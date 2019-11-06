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

namespace ViGraph { namespace Dataflow {

template<> inline
string get_module_type<MIDI::Event>() { return "midi"; }

template<> inline void set_from_json(MIDI::Event& event,
                                     const JSON::Value& json)
{
  const auto t = json["type"].as_str();
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
      event.channel = json["channel"].as_int();
      event.key = json["key"].as_int();
      event.value = json["velocity"].as_int();
      break;
    case MIDI::Event::Type::control_change:
      event.channel = json["channel"].as_int();
      event.key = json["control"].as_int();
      event.value = json["value"].as_int();
      break;
    case MIDI::Event::Type::none:
      break;
  }
}

template<> inline JSON::Value get_as_json(const MIDI::Event& event)
{
  JSON::Value json{JSON::Value::Type::OBJECT};
  switch (event.type)
  {
    case MIDI::Event::Type::note_on:
    case MIDI::Event::Type::note_off:
      json.put("type", event.type == MIDI::Event::Type::note_on ? "note-on"
                                                                : "note-off");
      json.put("channel", event.channel);
      json.put("key", event.key);
      json.put("velocity", event.value);
      break;
    case MIDI::Event::Type::control_change:
      json.put("type", "control-change");
      json.put("channel", event.channel);
      json.put("control", event.key);
      json.put("value", event.value);
      break;
    case MIDI::Event::Type::none:
      json.put("type", "none");
      break;
  }
  return json;
}

//==========================================================================
}} //namespaces

#endif // !__VIGRAPH_MIDI_MODULE_H
