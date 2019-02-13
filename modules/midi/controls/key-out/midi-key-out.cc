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
  uint8_t note{0};
  uint8_t velocity{0};
  bool wait{false};

  // Dynamic state
  bool done{false};
  bool triggered{false};

  // Control virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void tick(const TickData& td) override;
  // Automatically set wait flag if we are the target of something
  void notify_target_of(Element *) override { wait = true; }
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
  Element(module, config), Control(module, config, true)
{
  channel = config.get_attr_int("channel");
  auto ns = config["note"];
  if (!ns.empty())
  {
    auto n = MIDI::get_midi_note(ns);
    if (n < 0)
      note = Text::stoi(ns);
    else
      note = n;
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
// Set a control property
void MIDIKeyOutControl::set_property(const string& property,
                                    const SetParams& sp)
{
  if (property == "note")
  {
    if (sp.v.type == Value::Type::text)
      note = MIDI::get_midi_note(sp.v.s);
    else
      note = sp.v.d;
  }
  else if (property == "velocity")
  {
    velocity = sp.v.d;
  }
  else if (property == "trigger")
  {
    if (distributor)
    {
      auto event = MIDI::Event{MIDI::Event::Direction::out,
                               MIDI::Event::Type::note_on,
                               channel, note, velocity};
      distributor->handle_event(event);
    }
  }
  else if (property == "clear")
  {
    if (distributor)
    {
      auto event = MIDI::Event{MIDI::Event::Direction::out,
                               MIDI::Event::Type::note_off,
                               channel, note, velocity};
      distributor->handle_event(event);
    }
  }
  else if (property == "on" || property == "off")
  {
    if (distributor)
    {
      auto n = static_cast<uint8_t>(sp.v.d);
      if (sp.v.type == Value::Type::text)
        n = MIDI::get_midi_note(sp.v.s);
      const auto on = property == "on";
      auto event = MIDI::Event{MIDI::Event::Direction::out,
                               on ? MIDI::Event::Type::note_on
                                  : MIDI::Event::Type::note_off,
                               channel, n, velocity};
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
void MIDIKeyOutControl::tick(const TickData& /*td*/)
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
    { "channel", { {"MIDI channel (0=all)", "0"}, Value::Type::number,
                                                    "@channel" } },
    { "note", { {"Note number", "0"},
          Value::Type::number, "@note", true } },
    { "trigger", { "Note trigger", Value::Type::trigger, "@trigger", true }},
    { "clear",   { "Note release", Value::Type::trigger, "@clear", true }},
    { "on", { "Note on", Value::Type::number, "@on", true }},
    { "off", { "Note off", Value::Type::number, "@off", true }},
    { "velocity", { "Velocity", Value::Type::number, "@velocity", true }},
    { "wait",  { "Whether to wait for a trigger", Value::Type::number } }
  },
  {}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MIDIKeyOutControl, module)
