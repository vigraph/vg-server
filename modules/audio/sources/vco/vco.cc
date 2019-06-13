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
class VCOSource: public FragmentSource
{
public:
  // Configuration
  double freq;  // Hz
  Waveform::Type waveform = Waveform::Type::none;

private:
  vector<double> pulse_width{0.5};
  double theta = 0.0;
  enum State
  {
    disabled,
    enabled,
    completing
  } state = State::enabled;

  // Source/Element virtuals
  void tick(const TickData& td) override;
  void notify_target_of(const string& property) override;

public:
  using FragmentSource::FragmentSource;

  // Getters/Setters
  string get_note() const
  { return MIDI::get_midi_note(MIDI::get_midi_frequency_number(freq)); }
  void set_note(const string& note)
  { freq = MIDI::get_midi_frequency(MIDI::get_midi_number(note)); }
  int get_number() const { return MIDI::get_midi_frequency_number(freq); }
  void set_number(int number) { freq = MIDI::get_midi_frequency(number); }
  string get_waveform() const { return Waveform::get_name(waveform); }
  void set_waveform(const string& wave);
  double get_pulse_width() const { return pulse_width.back(); }
  void set_pulse_width(const vector<double>& pw);
  void start()
  {
    if (state != State::enabled)
    {
      if (state != State::completing)
        theta = 0.0;
      state = State::enabled;
    }
  }
  void stop() { if (state == State::enabled) state = State::completing; }
};

//--------------------------------------------------------------------------
// Set pulse width
void VCOSource::set_pulse_width(const vector<double>& pw)
{
  if (pw.empty())
    return;
  pulse_width = pw;
  for (auto& w: pulse_width)
    w = max(0.0, min(1.0, w));
}

//--------------------------------------------------------------------------
// Set waveform
void VCOSource::set_waveform(const string& name)
{
  if (!Waveform::get_type(name, waveform))
  {
    Log::Error log;
    log << "Unknown waveform type " << name << " in VCO '" << id << "'\n";
  }
}

//--------------------------------------------------------------------------
// If recipient of on/offs default to disabled
void VCOSource::notify_target_of(const string& property)
{
  if (property == "trigger")
    state = State::disabled;
}

//--------------------------------------------------------------------------
// Generate a fragment
void VCOSource::tick(const TickData& td)
{
  if (state == State::enabled || state == State::completing)
  {
    const auto nsamples = td.samples();
    auto fragment = new Fragment(td.t);  // mono
    auto& samples = fragment->waveforms[Speaker::front_center];
    samples.resize(nsamples);
    auto pw = pulse_width.begin();
    for (auto i=0u; i<nsamples; i++)
    {
      samples[i] = Waveform::get_value(waveform, *pw, theta);
      if (pw != pulse_width.end() - 1)
        ++pw;
      theta += freq/sample_rate;
      if (theta >= 1)
      {
        theta -= floor(theta); // Wrap to 0..1
        if (state == State::completing)
        {
          state = State::disabled;
          break;
        }
      }
    }
    const auto pulse_width_used = min(nsamples, pulse_width.size() - 1);
    pulse_width.erase(pulse_width.begin(),
                      pulse_width.begin() + pulse_width_used);

    // Send to output
    send(fragment);
  }
}

Dataflow::Module module
{
  "vco",
  "VCO",
  "Simple Voltage Controlled Oscillator",
  "audio",
  {
    { "freq",  { "Frequency (Hz)", Value::Type::number,
                 &VCOSource::freq, true } },
    { "note",  { "Note name (e.g C#4)", Value::Type::text,
                 { &VCOSource::get_note, &VCOSource::set_note },
                 true, true } },
    { "number",  { "MIDI note number (0-127)", Value::Type::number,
                   { &VCOSource::get_number, &VCOSource::set_number },
                   true, true } },
    { "wave",  { "Waveform type", Value::Type::text,
                 { &VCOSource::get_waveform, &VCOSource::set_waveform },
                 Waveform::get_names(), true } },
    { "pulse-width",  { "Pulse Width", Value::Type::number,
                        { &VCOSource::get_pulse_width,
                          &VCOSource::set_pulse_width}, true } },
    { "trigger", { "Trigger on", Value::Type::trigger,
                   &VCOSource::start,
                   true }},
    { "clear", { "Trigger off", Value::Type::trigger,
                 &VCOSource::stop,
                 true }},
  },
  {},  // no inputs
  { "Audio" }  // outputs
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(VCOSource, module)

