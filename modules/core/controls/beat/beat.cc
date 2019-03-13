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

  // Reset state
  void reset();

  // Control virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void pre_tick(const TickData& td) override;
  void notify_target_of(Element *, const string& property) override;
  void enable() override { reset(); }

public:
  // Construct
  BeatControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//    <beat interval="0.5"/>
// or <beat bpm="120"/>
// or <beat freq="2"/>
BeatControl::BeatControl(const Module *module, const XML::Element& config):
  Control(module, config)
{
  if (config.has_attr("interval"))
    interval = config.get_attr_real("interval");
  else if (config.has_attr("bpm") && config.get_attr_real("bpm") > 0)
    interval = 60.0 / config.get_attr_real("bpm");
  else if (config.has_attr("freq") && config.get_attr_real("freq") > 0)
    interval = 1.0 / config.get_attr_real("freq");
}

//--------------------------------------------------------------------------
// Reset state
void BeatControl::reset()
{
  last_trigger = -1000000.0;  // restart timebase
}

//--------------------------------------------------------------------------
// Automatically clear active flag if we are the start target of something
void BeatControl::notify_target_of(Element *, const string& property)
{
  if (property == "start")
    active = false;
}

//--------------------------------------------------------------------------
// Set a control property
void BeatControl::set_property(const string& property, const SetParams& sp)
{
  if (property == "start")
  {
    active = true;
    reset();
  }
  else if (property == "stop")
    active = false;
  else if (property == "interval")
    update_prop(interval, sp);
  else if (property == "bpm")
  {
    double bpm{0};
    update_prop(bpm, sp);
    if (bpm > 0) interval = 60.0 / bpm;
  }
  else if (property == "freq")
  {
    double freq{0};
    update_prop(freq, sp);
    if (freq > 0) interval = 1.0 / freq;
  }
}

//--------------------------------------------------------------------------
// Tick
void BeatControl::pre_tick(const TickData& td)
{
  if (!active) return;

  // Check interval
  if (td.t >= last_trigger + interval)
  {
    if (last_trigger < 0.0)
      last_trigger = td.t;      // Reset to now
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
    { "bpm", { "Beats per minute", Value::Type::number, true } },
    { "freq", { "Frequency in Hz", Value::Type::number, true } },
    { "start", { "Trigger to start", Value::Type::trigger, true } },
    { "stop",  { "Trigger to stop",  Value::Type::trigger, true } }
  },
  { { "", { "Trigger output", "trigger", Value::Type::trigger }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(BeatControl, module)
