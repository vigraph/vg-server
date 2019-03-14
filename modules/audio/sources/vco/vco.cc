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
  // Configuration
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
  double freq;  // Hz
  double pulse_width = 0.5;
  bool enabled = true;
  double theta = 0.0;

  // Parse waveform name
  bool parse_waveform(const string& name, Waveform& waveform);

  // Source/Element virtuals
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void set_property(const string& property, const SetParams& sp) override;
  void tick(const TickData& td) override;
  void notify_target_of(Element *, const string& property) override;

public:
  VCOSource(const Dataflow::Module *module, const XML::Element& config):
    Source(module, config) {}
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <vco> attributes:
//    freq: frequency in Hz
void VCOSource::configure(const File::Directory&,
                          const XML::Element& config)
{
  freq = config.get_attr_real("freq");
  pulse_width = max(0.0, min(1.0, config.get_attr_real("pulse-width",
                                                       pulse_width)));

  auto note = config["note"];
  if (!note.empty())
    freq = MIDI::get_midi_frequency(MIDI::get_midi_note(note));

  const string& wave = config["wave"];
  if (!parse_waveform(wave, waveform))
  {
    Log::Error log;
    log << "Unknown waveform type " << wave << " in VCO '" << id << "'\n";
  }
}

//--------------------------------------------------------------------------
// Parse waveform name
bool VCOSource::parse_waveform(const string& name, Waveform& waveform)
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
    return false;
  return true;
}

//--------------------------------------------------------------------------
// Set a control property
void VCOSource::set_property(const string& property, const SetParams& sp)
{
  if (property == "freq")
    update_prop(freq, sp);
  else if (property == "note")
    freq = MIDI::get_midi_frequency(MIDI::get_midi_note(sp.v.s));
  else if (property == "pulse-width")
    pulse_width = max(0.0, min(1.0, sp.v.d));
  else if (property == "wave")
    parse_waveform(sp.v.s, waveform);
  else if (property == "on")
  {
    const auto f = MIDI::get_midi_frequency(sp.v.d);
    enabled = true;
    freq = f;
  }
  else if (property == "trigger")
  {
    enabled = true;
  }
  else if (property == "off" || property == "clear")
  {
    enabled = false;
  }
}

//--------------------------------------------------------------------------
// If recipient of on/offs default to disabled
void VCOSource::notify_target_of(Element *, const string& property)
{
  if (property == "on")
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
          v = sin(theta*2*pi);
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
    { "freq",  { "Frequency (Hz)", Value::Type::number, "@freq", true } },
    { "note",  { "Note (e.g C#4)", Value::Type::text, "@note", true } },
    { "wave",  { "Waveform type (none, saw, sin, square, triangle, random)",
                 Value::Type::text, "@wave", true } },
    { "on", { "Note on", Value::Type::number, "@on", true }},
    { "off", { "Note off", Value::Type::number, "@off", true }},
    { "trigger", { "Trigger on", Value::Type::trigger, true }},
    { "clear", { "Trigger off", Value::Type::trigger, true }},
  },
  {},  // no inputs
  { "Audio" }  // outputs
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(VCOSource, module)

