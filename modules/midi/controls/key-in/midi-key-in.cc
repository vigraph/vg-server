//==========================================================================
// ViGraph dataflow module: midi/controls/midi-key-in/midi-key-in.cc
//
// Generic MIDI key input control
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include "../../midi-services.h"
#include "ot-web.h"
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
  shared_ptr<Distributor> distributor;

  // Control virtuals
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void enable() override;
  void disable() override;

  // Event observer implementation
  void handle(const ViGraph::MIDI::Event& event) override;

public:
  using Control::Control;

  // Property getter/setters
  string get_note() { return MIDI::get_midi_note(number); }
  void set_note(const string& note) { number = MIDI::get_midi_number(note); }
};

//--------------------------------------------------------------------------
// Configure from XML (once we have the engine)
void MIDIKeyInControl::configure(const File::Directory&,
                                 const XML::Element&)
{
  auto& engine = graph->get_engine();
  distributor = engine.get_service<Distributor>("midi-distributor");
  if (!distributor)
  {
    Log::Error log;
    log << "No <midi-distributor> service loaded\n";
  }
}

//--------------------------------------------------------------------------
// Enable - register for events
void MIDIKeyInControl::enable()
{
  Log::Detail log;
  log << "MIDI key enable on channel " << channel << endl;

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
}

//--------------------------------------------------------------------------
// Disable - deregister for events
void MIDIKeyInControl::disable()
{
  Log::Detail log;
  log << "MIDI key disable on channel " << channel << endl;

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
  "midi-key-in",
  "MIDI Key Input",
  "Generic MIDI Key Input",
  "midi",
  {
    { "channel", { "MIDI channel (0=all)", Value::Type::number,
      static_cast<int Element::*>(&MIDIKeyInControl::channel) } },
    { "number", { "Note number (-1=disable)", Value::Type::number,
      static_cast<int Element::*>(&MIDIKeyInControl::number), true } },
    { "note", {"Note (C3, A4#)", Value::Type::text,
    { static_cast<string (Element::*)()>(&MIDIKeyInControl::get_note),
      static_cast<void (Element::*)(const string&)>(
          &MIDIKeyInControl::set_note) },
      true, true } },
    { "min", { "Minimum note number (-1=disable)", Value::Type::number,
      static_cast<int Element::*>(&MIDIKeyInControl::min), true } },
    { "max", { "Maximum note number (-1=disable)", Value::Type::number,
      static_cast<int Element::*>(&MIDIKeyInControl::max), true } },
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
