//==========================================================================
// ViGraph dataflow module: audio/sources/vco/vco.cc
//
// Simple fixed-waveform Voltage Controlled Oscillator
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"
#include "vg-geometry.h"
#include "vg-waveform.h"
#include "vg-midi.h"
#include <cmath>

namespace {

using namespace ViGraph::Geometry;

//==========================================================================
// VCO source
class VCOSource: public Element
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

public:
  using Element::Element;

  // Configuration
  Input<Waveform::Type> waveform;
  Input<double> freq; // Hz
  Input<double> pulse_width;
  Output<double> output;
};

//--------------------------------------------------------------------------
// Generate a fragment
void VCOSource::tick(const TickData& td)
{
  const auto nsamples = td.samples();
  tick_iterate(nsamples, {}, tie(waveform, freq, pulse_width), tie(output),
               [&](Waveform::Type wf, double f, double pw, double& o)
  {
    switch (state)
    {
      case State::enabled:
      case State::completing:
        o = Waveform::get_value(wf, pw, theta);
        theta += f/sample_rate;
        if (theta >= 1)
        {
          theta -= floor(theta); // Wrap to 0..1
          if (state == State::completing)
            state = State::disabled;
        }
        break;
      case State::disabled:
        o = 0;
        break;
    }
  });
}

Dataflow::Module module
{
  "vco",
  "VCO",
  "audio",
  {},
  {
    { "wave",  &VCOSource::waveform },
    { "freq", &VCOSource::freq },
    { "pulse-width",  &VCOSource::pulse_width },
  },
  {
    { "output", &VCOSource::output },
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(VCOSource, module)

