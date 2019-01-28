//==========================================================================
// ViGraph dataflow module: audio/sources/wav/wav.cc
//
// Wav file source
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"
#include "vg-geometry.h"
#include <cmath>
#include <SDL_audio.h>

namespace {

using namespace ViGraph::Geometry;

//==========================================================================
// Wav source
class WavSource: public Source
{
  File::Path file;
  bool loop = false;
  vector<sample_t> samples;
  unsigned int channels = 2;
  double base_frequency = 0.0;
  double frequency = 0.0;

  //------------------------------------------------------------------------
  // Load a WAV into samples
  bool load_wav(const File::Path& f, vector<sample_t>& s);

  // Source/Element virtuals
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void set_property(const string& property, const SetParams& sp) override;
  void tick(const TickData& td) override;

public:
  WavSource(const Dataflow::Module *module, const XML::Element& config):
    Element(module, config), Source(module, config) {}
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <file> attributes:
//    path: path to sound file
void WavSource::configure(const File::Directory&,
                          const XML::Element& config)
{
  file = File::Path{config.get_attr("file")};
  loop = config.get_attr_bool("loop");
  base_frequency = config.get_attr_real("base-freq");
  frequency = config.get_attr_real("freq");

  if (frequency && !base_frequency)
  {
    Log::Error log;
    log << "Frequency requires that base frequency is also set in Wav '"
        << id << "'\n";
    return;
  }

  if (!load_wav(file, samples))
    return;

  Log::Detail dlog;
  dlog << "File loaded: " << file << "' in Wav '" << id << "'\n";
}

//--------------------------------------------------------------------------
// Load a WAV into samples
bool WavSource::load_wav(const File::Path& f, vector<sample_t>& s)
{
  if (!f.exists())
  {
    Log::Error log;
    log << "File not found: '" << f << "' in Wav '" << id << "'\n";
    return false;
  }

  SDL_AudioSpec spec;
  Uint8 *buffer = nullptr;
  Uint32 length = 0;
  if (!SDL_LoadWAV(f.c_str(), &spec, &buffer, &length))
  {
    Log::Error log;
    log << "File cannot be loaded: '" << f << "': " << SDL_GetError()
        << " in Wav '" << id << "'\n";
    return false;
  }

  s.resize(length / sizeof(sample_t));
  memcpy(&s[0], buffer, length);
  SDL_FreeWAV(buffer);

  channels = spec.channels;
  SDL_AudioCVT cvt;
  if (SDL_BuildAudioCVT(&cvt, spec.format, channels, spec.freq,
                              AUDIO_F32, channels, sample_rate) < 0)
  {
    Log::Error log;
    log << "Cannot prepare file for format conversion: '" << f << "': "
        << SDL_GetError() << " in Wav '" << id << "'\n";
    s.clear();
    return false;
  }

  if (cvt.needed)
  {
    const auto len_needed = length * cvt.len_mult;
    cvt.len = length;
    if (len_needed > length)
    {
      // We're going to need a bigger boat
      s.resize(len_needed / sizeof(sample_t));
    }
    cvt.buf = reinterpret_cast<Uint8 *>(&s[0]);
    if (SDL_ConvertAudio(&cvt))
    {
      Log::Error log;
      log << "Could not convert file to usable format: '" << f << "': "
          << SDL_GetError() << " in Wav '" << id << "'\n";
      s.clear();
      return false;
    }
    s.resize(cvt.len_ratio * cvt.len / sizeof(sample_t));
  }
  return true;
}

//--------------------------------------------------------------------------
// Set a control property
void WavSource::set_property(const string& property, const SetParams& sp)
{
  if (property == "loop")
    update_prop(loop, sp);
  else if (property == "freq")
    update_prop(frequency, sp);
}

//--------------------------------------------------------------------------
// Generate a fragment
void WavSource::tick(const TickData& td)
{
  if (samples.empty())
    return;

  auto step = frequency ? (frequency / base_frequency) : 1.0;
  auto pos = step * td.sample_pos(sample_rate);
  if (!loop && pos >= samples.size())
    return;

  const auto nsamples = td.samples(sample_rate);
  auto fragment = new Fragment(td.t, channels);
  fragment->waveform.reserve(nsamples * channels);

  for (auto i = 0u; i < nsamples; ++i, pos += step)
  {
    auto p = pos * channels;
    if (loop && p >= samples.size())
      p = fmod(p, samples.size());
    for (auto c = 0u; c < channels; ++c)
    {
      if (p >= samples.size())
      {
        fragment->waveform.push_back(0.0); // Pad with nothing
        continue;
      }

      auto fs = samples[p + c];
      auto cs = p + 1 > samples.size() ? 0.0 : samples[p + c + channels];
      auto is = fs + ((cs - fs) * ((pos * channels) - p));
      fragment->waveform.push_back(is);
    }
  }

  // Send to output
  send(fragment);
}

Dataflow::Module module
{
  "wav",
  "Wav",
  "Audio Wav Input",
  "audio",
  {
    { "file",  { "File path", Value::Type::file, "@file" } },
    { "loop",  { "Loop", Value::Type::boolean, "@loop", true } },
    { "base-freq",  { "Base frequency", Value::Type::number, "@base-freq" } },
    { "freq",  { "Frequency to play at", Value::Type::number, "@freq", true } },
  },
  {},  // no inputs
  { "Audio" }  // outputs
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(WavSource, module)
