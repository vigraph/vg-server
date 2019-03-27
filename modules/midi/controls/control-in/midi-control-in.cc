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
                            public Distributor::EventObserver
{
public:
  int channel{0};
  int number{0};
  double scale{1.0};
  double offset{0.0};

private:
  shared_ptr<Distributor> distributor;
  bool enabled = false;

  // Control virtuals
  void setup() override;

  // Event observer implementation
  void handle(const ViGraph::MIDI::Event& event) override;
  void notify_target_of(Element *, const string& property) override;

public:
  using Control::Control;

  // Event observer implementation
  void enable() override;
  void disable() override;
};

//--------------------------------------------------------------------------
// Setup
void MIDIControlInControl::setup()
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
void MIDIControlInControl::enable()
{
  if (distributor && !enabled)
  {
    Log::Detail log;
    log << "MIDI controller enable on channel " << channel
        << " controller " << number << endl;

    distributor->register_event_observer(
                                  ViGraph::MIDI::Event::Direction::in,
                                  channel, channel,
                                  ViGraph::MIDI::Event::Type::control_change,
                                  this);
    enabled = true;
  }
}

//--------------------------------------------------------------------------
// Disable - deregister for events
void MIDIControlInControl::disable()
{
  if (distributor && enabled)
  {
    Log::Detail log;
    log << "MIDI control disable on channel " << channel
        << " controller " << number << endl;

    distributor->deregister_event_observer(this);
    enabled = false;
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
    send(Dataflow::Value(scale*event.value/127.0+offset));
  }
}

//--------------------------------------------------------------------------
// If recipient of triggers default to disabled
void MIDIControlInControl::notify_target_of(Element *, const string& property)
{
  if (property == "enable")
    disable();
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
    { "channel", { "MIDI channel (0=all)", Value::Type::number,
      static_cast<int Element::*>(&MIDIControlInControl::channel), false } },
    { "number", { "Control number", Value::Type::number,
      static_cast<int Element::*>(&MIDIControlInControl::number), true } },
    { "scale",  { "Scale to apply to control value", Value::Type::number,
      static_cast<double Element::*>(&MIDIControlInControl::scale), true } },
    { "offset", { "Offset to apply to control value", Value::Type::number,
      static_cast<double Element::*>(&MIDIControlInControl::offset), true } },
    { "enable", { "Enable the control", Value::Type::trigger,
    static_cast<void (Element::*)()>(&MIDIControlInControl::enable), true } },
    { "disable", { "Disable the control", Value::Type::trigger,
    static_cast<void (Element::*)()>(&MIDIControlInControl::disable), true } },
  },
  { { "value", { "Control value", "value", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MIDIControlInControl, module)
