//==========================================================================
// ViGraph dataflow module: audio/sources/vco/vco.cc
//
// Simple fixed-waveform Voltage Controlled Oscillator
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"
#include "vg-geometry.h"
#include "vg-midi.h"
#include <cmath>

namespace {

using namespace ViGraph::Geometry;

//==========================================================================
// VCO source
class VCOSource: public Source
{
public:
  // Configuration
  double freq;  // Hz
  enum class Waveform
  {
    none,
    saw,
    sin,
    square,
    triangle,
    random
  };
  Waveform waveform;
  double pulse_width = 0.5;

private:
  double theta = 0.0;
  enum State
  {
    disabled,
    enabled,
    completing
  } state = State::enabled;

  // Parse waveform name
  bool parse_waveform(const string& name, Waveform& waveform);

  // Source/Element virtuals
  void tick(const TickData& td) override;
  void notify_target_of(const string& property) override;

public:
  using Source::Source;

  // Getters/Setters
  string get_note() const
  { return MIDI::get_midi_note(MIDI::get_midi_frequency_number(freq)); }
  void set_note(const string& note)
  { freq = MIDI::get_midi_frequency(MIDI::get_midi_number(note)); }
  int get_number() const { return MIDI::get_midi_frequency_number(freq); }
  void set_number(int number) { freq = MIDI::get_midi_frequency(number); }
  string get_waveform() const;
  void set_waveform(const string& wave);
  double get_pulse_width() const { return pulse_width; }
  void set_pulse_width(double pw) {pulse_width = max(0.0, min(1.0, pw)); }
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
// Get waveform
string VCOSource::get_waveform() const
{
  switch (waveform)
  {
    case Waveform::none: return "none";
    case Waveform::saw: return "saw";
    case Waveform::sin: return "sin";
    case Waveform::square: return "square";
    case Waveform::triangle: return "triangle";
    case Waveform::random: return "random";
  }
}

//--------------------------------------------------------------------------
// Set waveform
void VCOSource::set_waveform(const string& name)
{
  if (name.empty() || name == "none")
    waveform = Waveform::none;
  else if (name == "saw")
    waveform = Waveform::saw;
  else if (name == "sin")
    waveform = Waveform::sin;
  else if (name == "square")
    waveform = Waveform::square;
  else if (name == "triangle")
    waveform = Waveform::triangle;
  else if (name == "random")
    waveform = Waveform::random;
  else
  {
    waveform = Waveform::none;
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
    const auto nsamples = td.samples(sample_rate);
    auto fragment = new Fragment(td.t);  // mono
    auto& samples = fragment->waveforms[Speaker::front_center];
    samples.resize(nsamples);
    for (auto i=0u; i<nsamples; i++)
    {
      sample_t v;  // Value -1 .. 1

      switch (waveform)
      {
        case Waveform::none:
          v = 0.0;
          break;

        case Waveform::saw:
          if (theta < 0.5)
            v = theta * 2;
          else
            v = theta * 2 - 2;
          break;

        case Waveform::sin:
          v = theta < pulse_width ? sin(theta*pi/pulse_width)
                                  : sin((theta-1.0)*pi/(1.0-pulse_width));
          break;

        case Waveform::square:
          v = (theta < pulse_width) ? 1.0 : -1.0;
          break;

        case Waveform::triangle:
          if (theta < pulse_width / 2)
            v = theta / (pulse_width / 2);
          else if (theta >= (1 - (pulse_width / 2)))
            v = -1 + (theta - (1 - (pulse_width / 2))) / (pulse_width / 2);
          else
            v = 1 - (theta - (pulse_width / 2)) / ((1 - pulse_width) / 2);
          break;

        case Waveform::random:
          v = 2.0 * rand() / RAND_MAX - 1;
          break;
      }

      samples[i] = v;
      theta += freq/sample_rate;
      if (theta > 1)
      {
        theta -= floor(theta); // Wrap to 0..1
        if (state == State::completing)
        {
          state = State::disabled;
          break;
        }
      }
    }

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
    { "wave",  { "Waveform type (none, saw, sin, square, triangle, random)",
                 Value::Type::text,
                 { &VCOSource::get_waveform, &VCOSource::set_waveform },
                 true } },
    { "pulse-width",  { "Pulse Width", Value::Type::number,
                        &VCOSource::pulse_width, true } },
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

