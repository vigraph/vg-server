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
                        public Interface::EventObserver
{
  shared_ptr<Interface> interface;
  int channel{0};
  int note{0};

  // Control virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void enable() override;
  void disable() override;

  // Event observer implementation
  void handle(const ViGraph::MIDI::Event& event) override;

public:
  // Construct
  MIDIKeyInControl(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <midi-key-in channel="1" .../> - sends on/off for every key
//   <midi-key-in channel="1" note="48"/> - sends triggers for ON/OFF 48
MIDIKeyInControl::MIDIKeyInControl(const Dataflow::Module *module,
                                   const XML::Element& config):
  Element(module, config), Control(module, config)
{
  channel = config.get_attr_int("channel");
  note = config.get_attr_int("note");
}

//--------------------------------------------------------------------------
// Configure from XML (once we have the engine)
void MIDIKeyInControl::configure(const File::Directory&,
                                 const XML::Element&)
{
  auto& engine = graph->get_engine();
  interface = engine.get_service<Interface>("midi");
  if (!interface)
  {
    Log::Error log;
    log << "No MIDI service loaded\n";
  }
}

//--------------------------------------------------------------------------
// Set a control property
void MIDIKeyInControl::set_property(const string& property,
                                    const SetParams& sp)
{
  if (property == "note")
    update_prop_int(note, sp);
}

//--------------------------------------------------------------------------
// Enable - register for events
void MIDIKeyInControl::enable()
{
  Log::Detail log;
  log << "MIDI key enable on channel " << channel << endl;

  if (interface)
  {
    interface->register_event_observer(channel,
                                       ViGraph::MIDI::Event::Type::note_on,
                                       this);
    interface->register_event_observer(channel,
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

  if (interface)
    interface->deregister_event_observer(this);
}

//--------------------------------------------------------------------------
// Handle event
void MIDIKeyInControl::handle(const ViGraph::MIDI::Event& event)
{
  Log::Detail log;
  bool is_on = event.type==ViGraph::MIDI::Event::Type::note_on;

  if (!note || event.key == note)
  {
    log << "MIDI " << (int)event.channel << ": key " << (int)event.key
        << " " << (is_on?"ON":"OFF") << " @" << event.value << endl;
  }

  // Treat Note On with 0 velocity as off
  if (!event.value) is_on = false;

  // All notes on/off values if not specific
  if (!note)
    send(is_on?"on":"off", Dataflow::Value(event.key));

  // Specific triggers
  else if (event.key == note)
    send(is_on?"trigger":"clear", Dataflow::Value());

  // Send velocity separately if either is true, only for ON
  if (is_on && (!note || event.key == note))
    send("velocity", Dataflow::Value(event.value/127.0));
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
    { "channel", { {"MIDI channel (0=all)", "0"}, Value::Type::number,
                                                    "@channel" } },
    { "note", { {"Note number to trigger ON (0=disable)", "0"},
          Value::Type::number, "@note-on", true } },
    { "note-off", { {"Note number to trigger OFF (0=disable)", "0"},
          Value::Type::number, "@note-off", true } }
  },
  {
    { "trigger", { "Note trigger", "trigger", Value::Type::trigger }},
    { "clear",   { "Note release", "clear", Value::Type::trigger }},
    { "on", { "Note on", "on", Value::Type::number }},
    { "off", { "Note off", "off", Value::Type::number }},
    { "velocity", { "Velocity", "velocity", Value::Type::number }}
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MIDIKeyInControl, module)
