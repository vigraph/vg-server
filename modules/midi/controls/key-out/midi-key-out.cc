//==========================================================================
// ViGraph dataflow module: midi/controls/midi-key-out/midi-key-out.cc
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
// MIDIKeyOut control
class MIDIKeyOutControl: public Dataflow::Control
{
  shared_ptr<Distributor> distributor;

  // Dynamic state
  bool done{false};
  bool triggered{false};

  // Control virtuals
  void setup() override;
  void pre_tick(const TickData& td) override;
  void notify_target_of(const string& property) override;
  void enable() override;

public:
  int channel{0};
  int number{0};
  int velocity{0};
  bool wait{false};

  // Construct
  using Control::Control;

  // Getters/Setters
  string get_note() const { return MIDI::get_midi_note(number); }
  void set_note(const string& note) { number = MIDI::get_midi_number(note); }
  void on();
  void off();
};

//--------------------------------------------------------------------------
// Setup
void MIDIKeyOutControl::setup()
{
  distributor = graph->find_service<Distributor>("midi-distributor");
}

//--------------------------------------------------------------------------
// Automatically set wait flag if we are the trigger target of something
void MIDIKeyOutControl::notify_target_of(const string& property)
{
  if (property == "trigger")
    wait = true;
}

//--------------------------------------------------------------------------
// Send a key on
void MIDIKeyOutControl::on()
{
  if (distributor)
  {
    Log::Detail log;
    log << "MIDI OUT (" << id << ") channel " << (int)channel << ": key "
        << (int)number << " ON @" << (int)velocity << endl;
    auto event = MIDI::Event(MIDI::Event::Direction::out,
                             MIDI::Event::Type::note_on,
                             channel, number, velocity);
    distributor->handle_event(event);
  }
}

//--------------------------------------------------------------------------
// Send a key off
void MIDIKeyOutControl::off()
{
  if (distributor)
  {
    Log::Detail log;
    log << "MIDI OUT (" << id << ") channel " << (int)channel << ": key "
        << (int)number << " OFF @" << (int)velocity << endl;
    auto event = MIDI::Event(MIDI::Event::Direction::out,
                             MIDI::Event::Type::note_off,
                             channel, number, velocity);
    distributor->handle_event(event);
  }
}

//--------------------------------------------------------------------------
// Enable (reset)
void MIDIKeyOutControl::enable()
{
  triggered = done = false;
}

//--------------------------------------------------------------------------
// Tick
void MIDIKeyOutControl::pre_tick(const TickData& /*td*/)
{
  if (wait)
  {
    if (!triggered) return;
    triggered = false;
  }
  else
  {
    // Only run once
    if (done) return;
    done = true;
  }

  set_property("trigger", {});
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "midi-key-out",
  "MIDI Key Output",
  "Generic MIDI Key Output",
  "midi",
  {
    { "channel", { "MIDI channel (0=all)", Value::Type::number,
                   &MIDIKeyOutControl::channel, false } },
    { "number", { "Note number", Value::Type::number,
                  &MIDIKeyOutControl::number, true } },
    { "note", { "Note name", Value::Type::text,
                { &MIDIKeyOutControl::get_note, &MIDIKeyOutControl::set_note },
                true, true } },
    { "trigger", { "Note trigger", Value::Type::trigger,
                   &MIDIKeyOutControl::on, true } },
    { "clear",   { "Note release", Value::Type::trigger,
                   &MIDIKeyOutControl::off, true } },
    { "velocity", { "Velocity", Value::Type::number,
                    &MIDIKeyOutControl::velocity, true }},
    { "wait",  { "Whether to wait for a trigger", Value::Type::boolean,
                 &MIDIKeyOutControl::wait, false } }
  },
  {}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MIDIKeyOutControl, module)
