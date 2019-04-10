//==========================================================================
// ViGraph dataflow module: core/controls/lfo/lfo.cc
//
// LFO control to apply waveforms to properties
//
// <lfo wave="{saw|sin|square|triangle}"
//      once="yes|no"
//      wait="yes|no"
//      period="1.0" scale="1.0" offset="0.0" phase="0.0"
//       type="{real|integer|boolean}"
//       property="..."/>
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>
#include <stdlib.h>
#include "vg-geometry.h"
#include "vg-waveform.h"

namespace {

using namespace ViGraph::Geometry;

//==========================================================================
// LFO control
class LFOControl: public Dataflow::Control
{
  // Dynamic state
  double theta = 0.0;
  double last_t = 0.0;
  bool running{false};
  bool triggered{false};

  // Control virtuals
  void pre_tick(const TickData& td) override;
  void enable() override;

public:
  Waveform::Type waveform{Waveform::Type::saw};
  double pulse_width = 0.5;
  bool once{false};
  bool wait{false};
  double period{0.0};
  double scale{1.0};
  double offset{0.0};
  double phase{0.0};
  using Control::Control;

  // Property getter/setters
  string get_waveform() const { return Waveform::get_name(waveform); }
  void set_waveform(const string& wave);
  double get_pulse_width() const { return pulse_width; }
  void set_pulse_width(double pw) { pulse_width = max(0.0, min(1.0, pw)); }

  // Trigger function
  void trigger() { triggered = true; }
};

//--------------------------------------------------------------------------
// Set the wave from string
void LFOControl::set_waveform(const string& wave)
{
  if (!Waveform::get_type(wave, waveform))
    throw runtime_error("Unknown waveform type "+wave+" in "+id);
}

//--------------------------------------------------------------------------
// Enable (reset)
void LFOControl::enable()
{
  running = false;
  triggered = false;
}

//--------------------------------------------------------------------------
// Tick
void LFOControl::pre_tick(const TickData& td)
{
  const auto t = td.t;

  if (wait)
  {
    if (triggered)
    {
      last_t = t;
      running = true;
      triggered = false;
    }

    if (!running) return;
  }

  // Sanity check
  if (!period) return;

  theta += (t - last_t) / period;
  last_t = t;
  if (theta >= 1)
  {
    if (once)
      return;
    theta -= floor(theta);
  }

  // Get waveform value (-1..1)
  auto v = Waveform::get_value(waveform, pulse_width, theta);

  // Get raw (0..1) value
  v = (v + 1) / 2;

  // Adjust to configured output
  v = v*scale + offset;

  send(Dataflow::Value{v});
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "lfo",
  "LFO",
  "Low Frequency Oscillator",
  "core",
  {
    { "wave",  { { "Waveform", "sin" }, Value::Type::text,
                 { &LFOControl::get_waveform, &LFOControl::set_waveform },
                 Waveform::get_names(), true } },
    { "pulse-width",  { "Pulse Width", Value::Type::number,
                        &LFOControl::pulse_width, true } },
    { "once", { { "Run only one cycle", "false" }, Value::Type::boolean,
                &LFOControl::once, true } },
    { "wait", { { "Wait to be triggered", "false" }, Value::Type::boolean,
                &LFOControl::wait, true } },
    { "period", { { "Period in seconds", "1.0" }, Value::Type::number,
                  &LFOControl::period, true } },
    { "scale", { { "Scale (amplitude)", "1.0" }, Value::Type::number,
                 &LFOControl::scale, true } },
    { "offset", { "Baseline offset", Value::Type::number,
                  &LFOControl::offset, true } },
    { "phase", { "Phase shift (0..1)", Value::Type::number,
                 &LFOControl::phase, true } },
    { "trigger", { "Trigger to start", Value::Type::trigger,
                   &LFOControl::trigger, true } }
  },
  { { "value", { "Wave output", "value", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(LFOControl, module)
