//==========================================================================
// ViGraph dataflow module: core/controls/lfo/lfo.cc
//
// LFO control to apply waveforms to properties
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
  double scale{0.0};
  double offset{0.0};
  double phase{0.0};

  // Dynamic state
  bool running{false};
  bool triggered{false};
  timestamp_t trigger_time{0};

  // Internals
  void set_waveform(const string& wave);

  // Control virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void pre_tick(const TickData& td) override;
  void enable() override;

public:
  // Construct
  LFOControl(const Module *module, const XML::Element& config);
};

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
// Construct from XML
// <lfo wave="{saw|sin|square|triangle}"
//      once="yes|no"
//      wait="yes|no"
//      period="1.0" scale="1.0" offset="0.0" phase="0.0"
//       type="{real|integer|boolean}"
//       property="..."/>
LFOControl::LFOControl(const Module *module, const XML::Element& config):
  Control(module, config)
{
  set_waveform(config.get_attr("wave", "sin"));
  once = config.get_attr_bool("once");
  wait = config.get_attr_bool("wait");
  period = config.get_attr_real("period", 1.0);
  scale = config.get_attr_real("scale", 1.0);
  offset = config.get_attr_real("offset");
  phase = config.get_attr_real("phase");
}

//--------------------------------------------------------------------------
// Set a control property
void LFOControl::set_property(const string& property, const SetParams& sp)
{
  if (property == "period")
    update_prop(period, sp);
  else if (property == "scale")
    update_prop(scale, sp);
  else if (property == "offset")
    update_prop(offset, sp);
  else if (property == "phase")
    update_prop(phase, sp);
  else if (property == "trigger")
    triggered = true;
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
    { "wave",  { { "Waveform", "sin" },
          Value::Type::choice, "@wave",
          { "saw", "sin", "square", "triangle", "random" } } },
    { "once", { { "Run only one cycle", "false" },
          Value::Type::boolean } },
    { "wait", { { "Wait to be triggered", "false" },
          Value::Type::boolean } },
    { "period", { { "Period in seconds", "1.0" },
          Value::Type::number, true } },
    { "scale", { { "Scale (amplitude)", "1.0" },
          Value::Type::number, true } },
    { "offset", { "Baseline offset",
          Value::Type::number, true } },
    { "phase", { "Phase shift (0..1)",
          Value::Type::number, true } },
    { "trigger", { "Trigger to start",
          Value::Type::trigger, true } }
  },
  { { "", { "Wave output", "", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(LFOControl, module)
