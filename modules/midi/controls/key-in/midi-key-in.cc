//==========================================================================
// ViGraph dataflow module: midi/controls/midi-key-in/midi-key-in.cc
//
// Generic MIDI key input control
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include "../../midi-services.h"
#include "vg-midi.h"

using namespace ViGraph::Module::MIDI;

namespace {

//==========================================================================
// MIDIKeyIn control
class MIDIKeyInControl: public Dataflow::Control,
                        public Distributor::EventObserver
{
public:
  int channel{0};
  int number{-1};
  int min{-1};
  int max{-1};

private:
  // Control virtuals
  void enable() override;
  void disable() override;

  // Event observer implementation
  void handle(const ViGraph::MIDI::Event& event) override;

public:
  using Control::Control;

  // Property getter/setters
  string get_note() const { return MIDI::get_midi_note(number); }
  void set_note(const string& note) { number = MIDI::get_midi_number(note); }
};

//--------------------------------------------------------------------------
// Enable - register for events
void MIDIKeyInControl::enable()
{
  Log::Detail log;
  log << "MIDI key enable on channel " << channel << endl;

  auto distributor = graph->find_service<Distributor>("midi", "distributor");
  if (distributor)
  {
    distributor->register_event_observer(
                                      ViGraph::MIDI::Event::Direction::in,
                                      channel, channel,
                                      ViGraph::MIDI::Event::Type::note_on,
                                      this);
    distributor->register_event_observer(
                                      ViGraph::MIDI::Event::Direction::in,
                                      channel, channel,
                                      ViGraph::MIDI::Event::Type::note_off,
                                      this);
  }
  else
  {
    Log::Error elog;
    elog << "No MIDI distributor available for " << id << endl;
  }
}

//--------------------------------------------------------------------------
// Disable - deregister for events
void MIDIKeyInControl::disable()
{
  Log::Detail log;
  log << "MIDI key disable on channel " << channel << endl;

  auto distributor = graph->find_service<Distributor>("midi", "distributor");
  if (distributor)
    distributor->deregister_event_observer(this);
}

//--------------------------------------------------------------------------
// Handle event
void MIDIKeyInControl::handle(const ViGraph::MIDI::Event& event)
{
  if (number >= 0 && event.key != number)
    return;

  if (min >= 0 && event.key < min)
    return;

  if (max >= 0 && event.key > max)
    return;

  bool is_on = event.type==ViGraph::MIDI::Event::Type::note_on;

  Log::Detail log;
  log << "MIDI IN (" << id << ") channel " << (int)event.channel
      << ": key " << (int)event.key
      << " " << (is_on?"ON":"OFF") << " @" << event.value << endl;

  // Treat Note On with 0 velocity as off
  if (!event.value) is_on = false;

  // Send note value
  send("number", Dataflow::Value(event.key));

  // Send velocity separately if either is true, only for ON
  if (is_on)
    send("velocity", Dataflow::Value(event.value/127.0));

  // Specific triggers
  send(is_on?"trigger":"clear", Dataflow::Value());
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "key-in",
  "MIDI Key Input",
  "Generic MIDI Key Input",
  "midi",
  {
    { "channel", { "MIDI channel (0=all)", Value::Type::number,
                   &MIDIKeyInControl::channel } },
    { "number", { "Note number (-1=disable)", Value::Type::number,
                  &MIDIKeyInControl::number, true } },
    { "note", { "Note (C3, A4#)", Value::Type::text,
                { &MIDIKeyInControl::get_note, &MIDIKeyInControl::set_note },
                true, true } },
    { "min", { "Minimum note number (-1=disable)", Value::Type::number,
               &MIDIKeyInControl::min, true } },
    { "max", { "Maximum note number (-1=disable)", Value::Type::number,
               &MIDIKeyInControl::max, true } },
  },
  {
    { "trigger", { "Note trigger", "trigger", Value::Type::trigger }},
    { "clear",   { "Note release", "clear", Value::Type::trigger }},
    { "number", { "Note number", "number", Value::Type::number }},
    { "velocity", { "Velocity", "velocity", Value::Type::number }}
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MIDIKeyInControl, module)
