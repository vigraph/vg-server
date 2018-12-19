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
  int base{0};
  int modulus{0};

  // Control virtuals
  void configure(const XML::Element& config) override;

  // Event observer implementation
  void handle(ViGraph::MIDI::Event& event) override;

public:
  // Construct
  MIDIKeyInControl(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <midi-key-in base="24" modulus="12" ... target .../>
MIDIKeyInControl::MIDIKeyInControl(const Dataflow::Module *module,
                                   const XML::Element& config):
  Element(module, config), Control(module, config)
{
  channel = config.get_attr_int("channel");
  base = config.get_attr_int("base");
  modulus = config.get_attr_int("modulus");
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
  }
  else
  {
    Log::Error log;
    log << "No MIDI service loaded\n";
  }
}

//--------------------------------------------------------------------------
// Handle event
void MIDIKeyInControl::handle(ViGraph::MIDI::Event& event)
{
  Log::Detail log;
  log << "MIDI " << (int)event.channel << ": key " << (int)event.key
      << " ON @" << event.value << endl;
  int key = event.key-base;
  if (key >= 0)
  {
    if (modulus) key %= modulus;
    send(Dataflow::Value(key));
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "midi-key-in",
  "MIDI Key Input",
  "Generic MIDI Key Input",
  "linux",
  {
    { "channel", { {"MIDI channel (0=all)", "0"}, Value::Type::number,
                                                    "@channel" } },
    { "base",    { {"Base note value", "0"},      Value::Type::number,
                                                    "@base" } },
    { "modulus", { {"Note value modulus", "0"},   Value::Type::number,
                                                    "@modulus" } }
  },
  { { "", { "Key code", "key", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MIDIKeyInControl, module)
