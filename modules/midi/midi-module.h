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
string get_module_type<MIDI::Event>() { return "midi-event"; }

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
  event.channel = json["channel"].as_int();
  event.key = json["key"].as_int();
  event.value = json["value"].as_int();
}

template<> inline JSON::Value get_as_json(const MIDI::Event& event)
{
  JSON::Value json{JSON::Value::Type::OBJECT};
  switch (event.type)
  {
    case MIDI::Event::Type::note_on:
      json.put("type", "note-on");
      break;
    case MIDI::Event::Type::note_off:
      json.put("type", "note-off");
      break;
    case MIDI::Event::Type::control_change:
      json.put("type", "control-change");
      break;
    case MIDI::Event::Type::none:
      json.put("type", "none");
      break;
  }
  json.put("channel", event.channel);
  json.put("key", event.key);
  json.put("value", event.value);
  return json;
}

//==========================================================================
}} //namespaces

#endif // !__VIGRAPH_MIDI_MODULE_H
