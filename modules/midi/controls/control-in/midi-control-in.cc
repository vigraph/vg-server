//==========================================================================
// ViGraph dataflow module: midi/controls/midi-control-in/midi-control-in.cc
//
// Generic MIDI control input control
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include "../../midi-services.h"
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
  bool enabled = false;

  // Event observer implementation
  void handle(const ViGraph::MIDI::Event& event) override;
  void notify_target_of(const string& property) override;

public:
  using Control::Control;

  // Event observer implementation
  void enable() override;
  void disable() override;
};



//--------------------------------------------------------------------------
// Enable - register for events
void MIDIControlInControl::enable()
{
  auto distributor = graph->find_service<Distributor>("midi", "distributor");
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
  auto distributor = graph->find_service<Distributor>("midi", "distributor");
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
void MIDIControlInControl::notify_target_of(const string& property)
{
  if (property == "enable")
    disable();
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "control-in",
  "MIDI Control Input",
  "Generic MIDI Control Input",
  "midi",
  {
    { "channel", { "MIDI channel (0=all)", Value::Type::number,
                   &MIDIControlInControl::channel, false } },
    { "number", { "Control number", Value::Type::number,
                  &MIDIControlInControl::number, true } },
    { "scale",  { "Scale to apply to control value", Value::Type::number,
                  &MIDIControlInControl::scale, true } },
    { "offset", { "Offset to apply to control value", Value::Type::number,
                  &MIDIControlInControl::offset, true } },
    { "enable", { "Enable the control", Value::Type::trigger,
                  &MIDIControlInControl::enable, true } },
    { "disable", { "Disable the control", Value::Type::trigger,
                   &MIDIControlInControl::disable, true } },
  },
  { { "value", { "Control value", "value", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MIDIControlInControl, module)
