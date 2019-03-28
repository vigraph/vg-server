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
  bool enabled = true;

  // Parse waveform name
  bool parse_waveform(const string& name, Waveform& waveform);

  // Source/Element virtuals
  void tick(const TickData& td) override;
  void notify_target_of(Element *, const string& property) override;

public:
  using Source::Source;

  // Getters/Setters
  string get_note()
  { return MIDI::get_midi_note(MIDI::get_midi_frequency_number(freq)); }
  void set_note(const string& note)
  { freq = MIDI::get_midi_frequency(MIDI::get_midi_number(note)); }
  int get_number() { return MIDI::get_midi_frequency_number(freq); }
  void set_number(int number) { freq = MIDI::get_midi_frequency(number); }
  string get_waveform();
  void set_waveform(const string& wave);
  double get_pulse_width() { return pulse_width; }
  void set_pulse_width(double pw) {pulse_width = max(0.0, min(1.0, pw)); }
  void start() { enabled = true; }
  void stop() { enabled = false; }
};

//--------------------------------------------------------------------------
// Get waveform
string VCOSource::get_waveform()
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
void VCOSource::notify_target_of(Element *, const string& property)
{
  if (property == "trigger")
    enabled = false;
}

//--------------------------------------------------------------------------
// Generate a fragment
void VCOSource::tick(const TickData& td)
{
  if (enabled)
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
          v = theta*2 - 1;
        break;

        case Waveform::sin:
          v = theta < pulse_width ? sin(theta*pi/pulse_width)
                                  : sin((theta-1.0)*pi/(1.0-pulse_width));
        break;

        case Waveform::square:
          v = theta >= (1.0 - pulse_width) ? 1.0 : -1.0;
        break;

        case Waveform::triangle:
          v = (theta < 0.5 ? theta : 1-theta)*4-1;
        break;

        case Waveform::random:
          v = 2.0 * rand() / RAND_MAX - 1;
        break;
      }

      samples[i] = v;
      theta += freq/sample_rate;
      theta -= floor(theta); // Wrap to 0..1
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
                static_cast<double Element::*>(&VCOSource::freq), true } },
    { "note",  { "Note name (e.g C#4)", Value::Type::text,
     { static_cast<string (Element::*)()>(&VCOSource::get_note),
       static_cast<void (Element::*)(const string &)>(&VCOSource::set_note) },
                   true, true } },
    { "number",  { "MIDI note number (0-127)", Value::Type::number,
     { static_cast<int (Element::*)()>(&VCOSource::get_number),
       static_cast<void (Element::*)(int)>(&VCOSource::set_number) },
                   true, true } },
    { "wave",  { "Waveform type (none, saw, sin, square, triangle, random)",
                 Value::Type::text,
   { static_cast<string (Element::*)()>(&VCOSource::get_waveform),
     static_cast<void (Element::*)(const string&)>(&VCOSource::set_waveform) },
                 true } },
    { "pulse-width",  { "Pulse Width", Value::Type::number,
        static_cast<double Element::*>(&VCOSource::pulse_width), true } },
    { "trigger", { "Trigger on", Value::Type::trigger,
                   static_cast<void (Element::*)()>(&VCOSource::start),
                   true }},
    { "clear", { "Trigger off", Value::Type::trigger,
                 static_cast<void (Element::*)()>(&VCOSource::stop),
                 true }},
  },
  {},  // no inputs
  { "Audio" }  // outputs
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(VCOSource, module)

