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
  uint8_t channel{0};
  uint8_t number{0};
  uint8_t velocity{0};
  bool wait{false};

  // Dynamic state
  bool done{false};
  bool triggered{false};

  // Control virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void pre_tick(const TickData& td) override;
  void notify_target_of(Element *, const string& property) override;
  void enable() override;

public:
  // Construct
  MIDIKeyOutControl(const Dataflow::Module *module,
                    const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <midi-key-out channel="1" note="48"/> - sends triggers for ON/OFF 48
MIDIKeyOutControl::MIDIKeyOutControl(const Dataflow::Module *module,
                                     const XML::Element& config):
  Control(module, config, true)
{
  channel = config.get_attr_int("channel");
  number = config.get_attr_int("number");
  auto note = config["note"];
  if (!note.empty())
  {
    const auto n = MIDI::get_midi_number(note);
    if (n > 0)
      number = n;
  }
  velocity = config.get_attr_int("velocity");
  wait = config.get_attr_bool("wait");
}

//--------------------------------------------------------------------------
// Configure from XML (once we have the engine)
void MIDIKeyOutControl::configure(const File::Directory&,
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
// Automatically set wait flag if we are the trigger target of something
void MIDIKeyOutControl::notify_target_of(Element *, const string& property)
{
  if (property == "trigger")
    wait = true;
}

//--------------------------------------------------------------------------
// Set a control property
void MIDIKeyOutControl::set_property(const string& property,
                                    const SetParams& sp)
{
  if (property == "number")
  {
    number = sp.v.d;
  }
  else if (property == "note")
  {
    const auto n = MIDI::get_midi_number(sp.v.s);
    if (n > 0)
      number = n;
  }
  else if (property == "velocity")
  {
    velocity = sp.v.d;
  }
  else if (property == "trigger")
  {
    if (distributor)
    {
      Log::Detail log;
      log << "MIDI OUT (" << id << ") channel " << (int)channel << ": key "
          << (int)number << " ON @" << (int)velocity << endl;
      auto event = MIDI::Event{MIDI::Event::Direction::out,
                               MIDI::Event::Type::note_on,
                               channel, number, velocity};
      distributor->handle_event(event);
    }
  }
  else if (property == "clear")
  {
    if (distributor)
    {
      Log::Detail log;
      log << "MIDI OUT (" << id << ") channel " << (int)channel << ": key "
          << (int)number << " OFF @" << (int)velocity << endl;
      auto event = MIDI::Event{MIDI::Event::Direction::out,
                               MIDI::Event::Type::note_off,
                               channel, number, velocity};
      distributor->handle_event(event);
    }
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
    { "channel", { {"MIDI channel (0=all)", "0"}, Value::Type::number } },
    { "number", { {"Note number", "0"}, Value::Type::number, true } },
    { "note", { {"Note name", ""}, Value::Type::text, true } },
    { "trigger", { "Note trigger", Value::Type::trigger, true }},
    { "clear",   { "Note release", Value::Type::trigger, true }},
    { "velocity", { "Velocity", Value::Type::number, true }},
    { "wait",  { "Whether to wait for a trigger", Value::Type::number } }
  },
  {}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MIDIKeyOutControl, module)
