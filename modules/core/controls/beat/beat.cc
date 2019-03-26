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
 public:
  // Configured state
  Dataflow::timestamp_t interval{0.0};

 private:
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
  using Control::Control;

  // Property getter/setters
  double get_freq() { return (interval > 0) ? 1.0/interval : 0; }
  void set_freq(double f) { if (f > 0) interval = 1.0 / f; }
  double get_bpm() { return (interval > 0) ? 60.0/interval : 0; }
  void set_bpm(double bpm) { if (bpm > 0) interval = 60.0 / bpm; }

  // Trigger functions
  void trigger_start();
  void trigger_stop();
};

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
// Trigger a start
void BeatControl::trigger_start()
{
  active = true;
  reset();
}

//--------------------------------------------------------------------------
// Trigger stop
void BeatControl::trigger_stop()
{
  active = false;
}

//--------------------------------------------------------------------------
// Set a control property
// !!! Remove once setter functions called on trigger
void BeatControl::set_property(const string& property, const SetParams& sp)
{
  // Call up to set number properties
  Element::set_property(property, sp);

  // Set trigger properties manually
  if (property == "start")
    trigger_start();
  else if (property == "stop")
    trigger_stop();
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
    { "interval", { "Interval in seconds", Value::Type::number,
          static_cast<double Element::*>(&BeatControl::interval), true } },
    // The following two are aliases - settable, but not stored
    { "bpm", { "Beats per minute", Value::Type::number,
          { static_cast<double (Element::*)()>(&BeatControl::get_bpm),
          static_cast<void (Element::*)(double)>(&BeatControl::set_bpm) },
          true, true } },
    { "freq", { "Frequency in Hz", Value::Type::number,
          { static_cast<double (Element::*)()>(&BeatControl::get_freq),
            static_cast<void (Element::*)(double)>(&BeatControl::set_freq) },
          true, true } },

    // Trigger inputs
    { "start", { "Trigger to start", Value::Type::trigger,
          static_cast<void (Element::*)()>(&BeatControl::trigger_start),
          true } },
    { "stop",  { "Trigger to stop",  Value::Type::trigger,
          static_cast<void (Element::*)()>(&BeatControl::trigger_stop),
          true } }
  },
  { { "", { "Trigger output", "trigger", Value::Type::trigger }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(BeatControl, module)
