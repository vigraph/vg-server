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

namespace {

using namespace ViGraph::Geometry;

//==========================================================================
// LFO control
class LFOControl: public Dataflow::Control
{
  // Dynamic state
  bool running{false};
  bool triggered{false};
  timestamp_t trigger_time{0};

  // Control virtuals
  void pre_tick(const TickData& td) override;
  void enable() override;

public:
  enum class Waveform
  {
    saw,
    sin,
    square,
    triangle,
    random
  };

  Waveform waveform{Waveform::saw};
  bool once{false};
  bool wait{false};
  double period{0.0};
  double scale{1.0};
  double offset{0.0};
  double phase{0.0};
  using Control::Control;

  // Property getter/setters
  string get_waveform() const;
  void set_waveform(const string& wave);

  // Trigger function
  void trigger() { triggered = true; }
};

//--------------------------------------------------------------------------
// Get waveform name
string LFOControl::get_waveform() const
{
  switch (waveform)
  {
    case Waveform::saw:      return "saw";
    case Waveform::sin:      return "sin";
    case Waveform::square:   return "square";
    case Waveform::triangle: return "triangle";
    case Waveform::random:   return "random";
  }
}

//--------------------------------------------------------------------------
// Set the wave from string
void LFOControl::set_waveform(const string& wave)
{
  if (wave=="saw")
    waveform = Waveform::saw;
  else if (wave=="sin")
    waveform = Waveform::sin;
  else if (wave=="square")
    waveform = Waveform::square;
  else if (wave=="triangle")
    waveform = Waveform::triangle;
  else if (wave=="random")
    waveform = Waveform::random;
  else
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
  auto t = td.t;

  if (wait)
  {
    if (triggered)
    {
      trigger_time = t;
      running = true;
      triggered = false;
    }

    if (!running) return;

    t -= trigger_time;
  }

  // Divide by period to get slower timebase
  if (period > 0) t /= period;

  // If once, stop after first whole period
  if (once && t >= 1.0) return;

  // Add phase
  t += phase;

  // Get 0..1 repeating fraction
  t -= floor(t);

  // Get raw (0..1) value
  double v{0.0};
  switch (waveform)
  {
    case Waveform::saw:      v = t;                       break;
    case Waveform::sin:      v = 0.5+sin(t*2*pi)/2;       break;
    case Waveform::square:   v = t >= 0.5 ? 1.0 : 0.0;    break;
    case Waveform::triangle: v = (t < 0.5 ? t*2 : 2-t*2); break;
    case Waveform::random:   v = (double)rand()/RAND_MAX; break;
  }

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
    { "wave",  { { "Waveform", "sin" }, Value::Type::choice,
                 { &LFOControl::get_waveform, &LFOControl::set_waveform },
                 { "saw", "sin", "square", "triangle", "random" }, true } },
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
