//==========================================================================
// ViGraph dataflow module: audio/sources/oscillator/oscillator.cc
//
// Simple fixed-waveform Voltage Controlled Oscillator
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../core-module.h"
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
  } state = State::enabled;

  // Source/Element virtuals
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
  Output<double> output;
  Output<double> control;
};

//--------------------------------------------------------------------------
// Generate a fragment
void OscillatorSource::tick(const TickData& td)
{
  sample_iterate(td.nsamples, {}, tie(waveform, freq, pulse_width),
                 tie(output, control),
                 [&](Waveform::Type wf, double f, double pw,
                     double& o, double& c)
  {
    switch (state)
    {
      case State::enabled:
      case State::completing:
        o = Waveform::get_value(wf, pw, theta);
        c = (o + 1) / 2;
        theta += f / td.sample_rate;
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
  },
  {
    { "output", &OscillatorSource::output },
    { "control", &OscillatorSource::control },
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(OscillatorSource, module)
