//==========================================================================
// ViGraph dataflow module: midi/controls/midi-control-in/midi-control-in.cc
//
// Generic MIDI control input control
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
// MIDIControlIn control
class MIDIControlInControl: public Dataflow::Control,
                        public Interface::EventObserver
{
  shared_ptr<Interface> interface;
  int channel{0};
  int number{0};

  // Control virtuals
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;

  // Event observer implementation
  void handle(const ViGraph::MIDI::Event& event) override;

public:
  // Construct
  MIDIControlInControl(const Dataflow::Module *module,
                       const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <midi-control-in ... target .../>
MIDIControlInControl::MIDIControlInControl(const Dataflow::Module *module,
                                           const XML::Element& config):
  Element(module, config), Control(module, config)
{
  channel = config.get_attr_int("channel");
  number = config.get_attr_int("number");
}

//--------------------------------------------------------------------------
// Configure from XML (once we have the engine)
void MIDIControlInControl::configure(const File::Directory&,
                                     const XML::Element&)
{
  auto& engine = graph->get_engine();
  interface = engine.get_service<Interface>("midi");
  if (interface)
  {
    interface->register_event_observer(channel,
                                  ViGraph::MIDI::Event::Type::control_change,
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
void MIDIControlInControl::handle(const ViGraph::MIDI::Event& event)
{
  if (number == event.key)
  {
    Log::Detail log;
    log << "MIDI " << (int)event.channel << ": control " << (int)event.key
        << " -> " << event.value << endl;
    send(Dataflow::Value(event.value));
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "midi-control-in",
  "MIDI Control Input",
  "Generic MIDI Control Input",
  "midi",
  {
    { "channel", { {"MIDI channel (0=all)", "0"}, Value::Type::number,
                                                    "@channel" } },
    { "number", { {"Control number", "0"}, Value::Type::number,
                                                    "@number" } }
  },
  { { "", { "Control value", "value", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MIDIControlInControl, module)
