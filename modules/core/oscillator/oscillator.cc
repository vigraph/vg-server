//==========================================================================
// ViGraph dataflow module: core/oscillator/oscillator.cc
//
// Simple fixed-waveform Voltage Controlled Oscillator
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../core-module.h"
#include "vg-geometry.h"

namespace {

using namespace ViGraph::Geometry;

//==========================================================================
// Oscillator source
class OscillatorSource: public SimpleElement
{
private:
  double theta = 0.0;
  enum State
  {
    disabled,
    enabled,
    completing
  } state = State::disabled;   // Note we go enabled if start not connected

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  OscillatorSource *create_clone() const override
  {
    return new OscillatorSource{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<Waveform::Type> waveform;
  Input<double> freq{1}; // Hz
  Input<double> pulse_width{0.5};

  // Triggers
  Input<double> start{0.0};    // Trigger to start
  Input<double> stop{0.0};     // Trigger to stop

  // Output
  Output<double> output;
  Output<double> control;
};

//--------------------------------------------------------------------------
// Generate a fragment
void OscillatorSource::tick(const TickData& td)
{
  const auto sample_rate = max(output.get_sample_rate(),
                               control.get_sample_rate());
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(nsamples, {}, tie(waveform, freq, pulse_width, start, stop),
                 tie(output, control),
                 [&](Waveform::Type wf, double f, double pw,
                     double _start, double _stop,
                     double& o, double& c)
  {
    if (_stop)
    {
      if (state == State::enabled)
        state = State::completing;
    }
    else if (_start || !start.connected())
    {
      if (state == State::disabled)
      {
        state = State::enabled;
        theta = 0.0;
      }
    }

    switch (state)
    {
      case State::enabled:
      case State::completing:
        o = Waveform::get_value(wf, pw, theta);
        c = (o + 1) / 2;
        theta += f / sample_rate;
        if (theta >= 1)
        {
          theta -= floor(theta); // Wrap to 0..1
          if (state == State::completing)
            state = State::disabled;
        }
        break;
      case State::disabled:
        o = 0;
        c = 0;
        break;
    }
  });
}

Dataflow::SimpleModule module
{
  "oscillator",
  "Oscillator",
  "core",
  {},
  {
    { "wave",         &OscillatorSource::waveform },
    { "freq",         &OscillatorSource::freq },
    { "pulse-width",  &OscillatorSource::pulse_width },
    { "start",        &OscillatorSource::start },
    { "stop",         &OscillatorSource::stop },
  },
  {
    { "output", &OscillatorSource::output },
    { "control", &OscillatorSource::control },
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(OscillatorSource, module)
