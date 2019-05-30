//==========================================================================
// ViGraph dataflow module: dmx/controls/dmx-channel-in/dmx-channel-in.cc
//
// Generic DMX channel input control
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include "../../dmx-services.h"

using namespace ViGraph::Module::DMX;

namespace {

//==========================================================================
// DMXChannelIn control
class DMXChannelInControl: public Dataflow::Control,
                           public Distributor::EventObserver
{
public:
  int universe{0};
  int channel{0};
  double scale{1.0};
  double offset{0.0};

private:
  bool enabled = false;

  // Event observer implementation
  void handle(unsigned universe, unsigned channel, dmx_value_t value) override;
  void notify_target_of(const string& property) override;

public:
  using Control::Control;

  // Event observer implementation
  void enable() override;
  void disable() override;
};

//--------------------------------------------------------------------------
// Enable - register for events
void DMXChannelInControl::enable()
{
  auto distributor = graph->find_service<Distributor>("dmx", "distributor");
  if (distributor && !enabled)
  {
    Log::Detail log;
    log << "DMX IN controller enable on universe " << universe
        << " channel " << channel << endl;

    distributor->register_event_observer(Direction::in,
                                         universe, universe, channel, channel,
                                         this);
    enabled = true;
  }
}

//--------------------------------------------------------------------------
// Disable - deregister for events
void DMXChannelInControl::disable()
{
  auto distributor = graph->find_service<Distributor>("dmx", "distributor");
  if (distributor && enabled)
  {
    Log::Detail log;
    log << "DMX IN control disable on universe " << universe
        << " channel " << channel << endl;

    distributor->deregister_event_observer(this);
    enabled = false;
  }
}

//--------------------------------------------------------------------------
// Handle event
void DMXChannelInControl::handle(unsigned universe, unsigned chan,
                                 dmx_value_t value)
{
  if (static_cast<int>(chan) == channel)
  {
    Log::Detail log;
    log << "DMX IN " << universe << ": channel " << channel
        << " -> " << static_cast<int>(value) << endl;
    send(Dataflow::Value(scale * value / 255.0 + offset));
  }
}

//--------------------------------------------------------------------------
// If recipient of triggers default to disabled
void DMXChannelInControl::notify_target_of(const string& property)
{
  if (property == "enable")
    disable();
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "channel-in",
  "DMX Channel Input",
  "Generic DMX Channel Input",
  "dmx",
  {
    { "universe", { "DMX universe", Value::Type::number,
                    &DMXChannelInControl::universe, false } },
    { "channel", { "Control channel", Value::Type::number,
                   &DMXChannelInControl::channel, true } },
    { "scale",  { "Scale to apply to control value", Value::Type::number,
                  &DMXChannelInControl::scale, true } },
    { "offset", { "Offset to apply to control value", Value::Type::number,
                  &DMXChannelInControl::offset, true } },
    { "enable", { "Enable the control", Value::Type::trigger,
                  &DMXChannelInControl::enable, true } },
    { "disable", { "Disable the control", Value::Type::trigger,
                   &DMXChannelInControl::disable, true } },
  },
  { { "output", { "Channel value", "value", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(DMXChannelInControl, module)
