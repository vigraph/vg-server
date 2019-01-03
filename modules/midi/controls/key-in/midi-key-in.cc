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

  // Control virtuals
  void configure(const XML::Element& config) override;

  // Event observer implementation
  void handle(const ViGraph::MIDI::Event& event) override;

public:
  // Construct
  MIDIKeyInControl(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <midi-key-in channel="1" target .../>
MIDIKeyInControl::MIDIKeyInControl(const Dataflow::Module *module,
                                   const XML::Element& config):
  Element(module, config), Control(module, config)
{
  channel = config.get_attr_int("channel");
}

//--------------------------------------------------------------------------
// Configure from XML (once we have the engine)
void MIDIKeyInControl::configure(const XML::Element&)
{
  auto& engine = graph->get_engine();
  interface = engine.get_service<Interface>("midi");
  if (interface)
  {
    interface->register_event_observer(channel,
                                       ViGraph::MIDI::Event::Type::note_on,
                                       this);
    interface->register_event_observer(channel,
                                       ViGraph::MIDI::Event::Type::note_off,
                                       this);
  }
  else
  {
    Log::Error log;
    log << "No MIDI service loaded\n";
  }
}

//--------------------------------------------------------------------------
// Handle event
void MIDIKeyInControl::handle(const ViGraph::MIDI::Event& event)
{
  Log::Detail log;
  bool is_on = event.type==ViGraph::MIDI::Event::Type::note_on;
  log << "MIDI " << (int)event.channel << ": key " << (int)event.key
      << " " << (is_on?"ON":"OFF") << " @" << event.value << endl;

  send(Dataflow::Value(is_on ? event.key : -event.key));
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
  },
  { { "", { "Key code", "key", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MIDIKeyInControl, module)
