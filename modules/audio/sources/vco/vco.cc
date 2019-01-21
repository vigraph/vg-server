//==========================================================================
// ViGraph dataflow module: audio/sources/vco/vco.cc
//
// Simple fixed-waveform Voltage Controlled Oscillator
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"
#include "vg-geometry.h"
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

  // Source/Element virtuals
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void set_property(const string& property, const SetParams& sp) override;
  void tick(const TickData& td) override;

public:
  VCOSource(const Dataflow::Module *module, const XML::Element& config):
    Element(module, config), Source(module, config) {}
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <vco> attributes:
//    freq: frequency in Hz
void VCOSource::configure(const File::Directory&,
                          const XML::Element& config)
{
  freq = config.get_attr_real("freq");

  const string& wave = config["wave"];
  if (wave.empty() || wave=="none")
    waveform = Waveform::none;
  else if (wave=="saw")
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
  {
    Log::Error log;
    log << "Unknown waveform type " << wave << " in VCO '" << id << "'\n";
  }
}

//--------------------------------------------------------------------------
// Set a control property
void VCOSource::set_property(const string& property, const SetParams& sp)
{
  if (property == "freq")
    update_prop(freq, sp);
}

//--------------------------------------------------------------------------
// Generate a fragment
void VCOSource::tick(const TickData& td)
{
  const auto last_tick_total = static_cast<int>(
      floor(td.interval.seconds() * (td.n) * sample_rate));
  const auto tick_total = static_cast<int>(
      floor(td.interval.seconds() * (td.n + 1) * sample_rate));
  const auto nsamples = tick_total - last_tick_total;

  auto fragment = new Fragment(td.t);  // mono
  fragment->waveform.reserve(nsamples);
  auto theta = td.t * freq;
  theta -= floor(theta); // Wrap to 0..1
  for(int i=0; i<nsamples; i++)
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
        v = theta >= 0.5 ? 1.0 : -1.0;
      break;

      case Waveform::triangle:
        v = (theta < 0.5 ? theta : 1-theta)*4-1;
      break;

      case Waveform::random:
        v = 2.0 * rand() / RAND_MAX - 1;
      break;
    }

    fragment->waveform.push_back(v);
    theta += freq/sample_rate;
    theta -= floor(theta); // Wrap to 0..1
  }

  // Send to output
  send(fragment);
}

Dataflow::Module module
{
  "vco",
  "VCO",
  "Simple Voltage Controlled Oscillator",
  "audio",
  {
    { "freq",  { "Frequency (Hz)", Value::Type::number, "@freq", true } }
  },
  {},  // no inputs
  { "Audio" }  // outputs
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(VCOSource, module)

