//==========================================================================
// ViGraph dataflow module: core/controls/beat/beat.cc
//
// Control to provide repetitive triggers at a defined interval
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>
#include <stdlib.h>

namespace {

//==========================================================================
// Beat control
class BeatControl: public Dataflow::Control
{
  // Configured state
  Dataflow::timestamp_t interval{0.0};

  // Dynamic state
  bool active{true};  // unless cancelled by trigger connection
  Dataflow::timestamp_t last_trigger{-1000000.0};

  // Control virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void tick(Dataflow::timestamp_t t) override;
  // Disable auto-active if we are the target of something
  void notify_target_of(Element *) override { active = false; }

public:
  // Construct
  BeatControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
// <beat interval="0.1"/>
BeatControl::BeatControl(const Module *module, const XML::Element& config):
  Element(module, config), Control(module, config)
{
  interval = config.get_attr_real("interval");
}

//--------------------------------------------------------------------------
// Set a control property
void BeatControl::set_property(const string& property, const SetParams& sp)
{
  if (property == "start")
  {
    active = true;
    last_trigger = -1000000.0;  // restart timebase
  }
  else if (property == "stop")
    active = false;
  else if (property == "interval")
    update_prop(interval, sp);
}

//--------------------------------------------------------------------------
// Tick
void BeatControl::tick(Dataflow::timestamp_t t)
{
  if (!active) return;

  // Check interval
  if (t >= last_trigger + interval)
  {
    if (last_trigger < 0.0)
      last_trigger = t;         // Reset to now
    else
      last_trigger += interval; // Maintain constant timebase
  }
  else return;  // Wait

  send(SetParams{Dataflow::Value{}});  // trigger
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "beat",
  "Beat",
  "Regular beat pulse",
  "core",
  {
    { "interval", { "Interval in seconds", Value::Type::number, true } },
    { "start", { "Trigger to start", Value::Type::trigger, true } },
    { "stop",  { "Trigger to stop",  Value::Type::trigger, true } }
  },
  { { "", { "Trigger output", "trigger", Value::Type::trigger }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(BeatControl, module)
