//==========================================================================
// ViGraph dataflow module: midi/controls/midi-control-out/midi-control-out.cc
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
// MIDIControlOut control
class MIDIControlOutControl: public Dataflow::Control
{
public:
  int channel{0};
  int number{0};

private:
  bool enabled = false;

  // Event observer implementation
  void notify_target_of(const string& property) override;

public:
  using Control::Control;

  //------------------------------------------------------------------------
  // Set value
  void set_value(double value);

  // Event observer implementation
  void enable() override;
  void disable() override;
};

//--------------------------------------------------------------------------
// Enable - register for events
void MIDIControlOutControl::enable()
{
  if (!enabled)
  {
    Log::Detail log;
    log << "MIDI OUT controller enable on channel " << channel
        << " controller " << number << endl;
    enabled = true;
  }
}

//--------------------------------------------------------------------------
// Disable - deregister for events
void MIDIControlOutControl::disable()
{
  if (enabled)
  {
    Log::Detail log;
    log << "MIDI OUT control disable on channel " << channel
        << " controller " << number << endl;
    enabled = false;
  }
}

//--------------------------------------------------------------------------
// Set value
void MIDIControlOutControl::set_value(double value)
{
  if (enabled)
  {
    auto distributor = graph->find_service<Distributor>("midi", "distributor");
    if (distributor)
    {
      value *= 127.0;
#if OBTOOLS_LOG_DEBUG
      Log::Debug log;
      log << "MIDI OUT " << channel << ": control " << number
          << " -> " << static_cast<int>(value) << endl;
#endif
      auto event = MIDI::Event(MIDI::Event::Direction::out,
                               MIDI::Event::Type::control_change,
                               channel, number, value);
      distributor->handle_event(event);
    }
  }
}

//--------------------------------------------------------------------------
// If recipient of triggers default to disabled
void MIDIControlOutControl::notify_target_of(const string& property)
{
  if (property == "enable")
    disable();
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "control-out",
  "MIDI Control Output",
  "Generic MIDI Control Output",
  "midi",
  {
    { "channel", { "MIDI channel (0=all)", Value::Type::number,
                   &MIDIControlOutControl::channel, false } },
    { "number", { "Control number", Value::Type::number,
                  &MIDIControlOutControl::number, true } },
    { "value",  { "Control value", Value::Type::number,
                  { &MIDIControlOutControl::set_value }, true } },
    { "enable", { "Enable the control", Value::Type::trigger,
                  &MIDIControlOutControl::enable, true } },
    { "disable", { "Disable the control", Value::Type::trigger,
                   &MIDIControlOutControl::disable, true } },
  },
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MIDIControlOutControl, module)
