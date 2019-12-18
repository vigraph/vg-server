//==========================================================================
// ViGraph dataflow module: core/oscillator/oscillator.cc
//
// Simple fixed-waveform Voltage Controlled Oscillator
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../core-module.h"
#include "../../waveform.h"
#include "vg-geometry.h"

namespace {

using namespace ViGraph::Geometry;

//==========================================================================
// Oscillator source
class OscillatorSource: public SimpleElement
{
private:
  Number theta = 0.0;
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
  Input<Number> period{1}; // sec
  Input<Number> pulse_width{0.5};
  Input<Number> phase{0};

  // Triggers
  Input<Trigger> start{0.0};    // Trigger to start
  Input<Trigger> stop{0.0};     // Trigger to stop

  // Output
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Generate a fragment
void OscillatorSource::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {},
                 tie(waveform, period, pulse_width, phase, start, stop),
                 tie(output),
                 [&](Waveform::Type wf, Number period, Number pw, Number phase,
                     Trigger _start, Trigger _stop,
                     Number& output)
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
      {
        auto tp = theta+phase;
        output = (Waveform::get_value(wf, pw, tp-floor(tp))+1)/2;
        if (period > 0) theta += 1 / period / sample_rate;
        if (theta >= 1)
        {
          theta -= floor(theta); // Wrap to 0..1
          if (state == State::completing)
            state = State::disabled;
        }
        break;
      }

      case State::disabled:
        output = 0;
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
    { "period",       &OscillatorSource::period },
    { "pulse-width",  &OscillatorSource::pulse_width },
    { "phase",        &OscillatorSource::phase },
    { "start",        &OscillatorSource::start },
    { "stop",         &OscillatorSource::stop },
  },
  {
    { "output", &OscillatorSource::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(OscillatorSource, module)
