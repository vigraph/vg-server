//==========================================================================
// ViGraph dataflow module: core/oscillator/oscillator.cc
//
// Simple fixed-waveform Voltage Controlled Oscillator
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../audio-module.h"
#include "../../waveform.h"
#include "vg-music.h"

namespace {

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
  Input<Number> note{0};   // 1 per octave, 0 = Middle C
  Input<Number> octave{0}; // 1 per octave, added to above
  Input<Number> detune{0}; // 1 per semi tone, added/12 to above
  Input<Number> pulse_width{0.5};

  // Triggers
  Input<Trigger> start{0};    // Trigger to start
  Input<Trigger> stop{0};     // Trigger to stop

  // Output
  Output<AudioData> output;
};

//--------------------------------------------------------------------------
// Generate a fragment
void OscillatorSource::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {},
                 tie(waveform, note, octave, detune, pulse_width, start, stop),
                 tie(output),
                 [&](Waveform::Type waveform, Number note, Number octave,
                     Number detune, Number pulse_width,
                     Trigger _start, Trigger _stop,
                     AudioData& output)
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

    output.nchannels = 1;

    switch (state)
    {
      case State::enabled:
      case State::completing:
      {
        output.channels[0] =
          Waveform::get_value(waveform, pulse_width, theta-floor(theta));
        auto cv = note + octave + detune/12;
        auto freq = Music::cv_to_frequency(cv);
        theta += freq / sample_rate;
        if (theta >= 1)
        {
          theta -= floor(theta); // Wrap to 0..1
          if (state == State::completing)
            state = State::disabled;
        }
        break;
      }

      case State::disabled:
        output.channels[0] = 0;
        break;
    }
  });
}

Dataflow::SimpleModule module
{
  "oscillator",
  "Audio Oscillator",
  "audio",
  {},
  {
    { "wave",         &OscillatorSource::waveform },
    { "note",         &OscillatorSource::note },
    { "octave",       &OscillatorSource::octave },
    { "detune",       &OscillatorSource::detune },
    { "pulse-width",  &OscillatorSource::pulse_width },
    { "start",        &OscillatorSource::start },
    { "stop",         &OscillatorSource::stop },
  },
  {
    { "output", &OscillatorSource::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(OscillatorSource, module)
