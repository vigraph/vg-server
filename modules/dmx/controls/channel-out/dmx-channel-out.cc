//==========================================================================
// ViGraph dataflow module: dmx/controls/dmx-channel-out/dmx-channel-out.cc
//
// Generic DMX channel output control
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include "../../dmx-services.h"

using namespace ViGraph::Module::DMX;

namespace {

//==========================================================================
// DMXChannelOut control
class DMXChannelOutControl: public Dataflow::Control
{
public:
  int universe{0};
  int channel{0};

private:
  shared_ptr<Distributor> distributor;
  bool enabled = false;

  // Control virtuals
  void setup() override;

  // Event observer implementation
  void notify_target_of(const string& property) override;

public:
  using Control::Control;

  // Set the value
  void set_value(double value);

  // Event observer implementation
  void enable() override;
  void disable() override;
};

//--------------------------------------------------------------------------
// Setup
void DMXChannelOutControl::setup()
{
  distributor = graph->find_service<Distributor>("dmx:distributor");
}

//--------------------------------------------------------------------------
// Enable - register for events
void DMXChannelOutControl::enable()
{
  if (distributor && !enabled)
  {
    Log::Detail log;
    log << "DMX OUT controller enable on universe " << universe
        << " channel " << channel << endl;
    enabled = true;
  }
}

//--------------------------------------------------------------------------
// Disable - deregister for events
void DMXChannelOutControl::disable()
{
  if (distributor && enabled)
  {
#if OBTOOLS_LOG_DEBUG
    Log::Debug log;
    log << "DMX OUT control disable on universe " << universe
        << " channel " << channel << endl;
#endif
    enabled = false;
  }
}

//--------------------------------------------------------------------------
// Set value
void DMXChannelOutControl::set_value(double value)
{
  if (distributor && enabled)
  {
    value *= 255.0;
#if OBTOOLS_LOG_DEBUG
    Log::Debug log;
    log << "DMX OUT " << universe << ": channel " << channel
        << " -> " << static_cast<int>(value) << endl;
#endif
    distributor->handle_event(Direction::out, universe, channel, value);
  }
}

//--------------------------------------------------------------------------
// If recipient of triggers default to disabled
void DMXChannelOutControl::notify_target_of(const string& property)
{
  if (property == "enable")
    disable();
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "channel-out",
  "DMX Channel Output",
  "Generic DMX Channel Output",
  "dmx",
  {
    { "universe", { "DMX universe", Value::Type::number,
                    &DMXChannelOutControl::universe, false } },
    { "channel", { "Control channel", Value::Type::number,
                   &DMXChannelOutControl::channel, true } },
    { "value", { "Channel value", Value::Type::number,
                 { &DMXChannelOutControl::set_value }, true } },
    { "enable", { "Enable the control", Value::Type::trigger,
                  &DMXChannelOutControl::enable, true } },
    { "disable", { "Disable the control", Value::Type::trigger,
                   &DMXChannelOutControl::disable, true } },
  },
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(DMXChannelOutControl, module)
