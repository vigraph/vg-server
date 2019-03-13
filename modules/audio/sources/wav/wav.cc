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
using namespace ViGraph::Module::Audio;

//==========================================================================
// Wav source
class WavSource: public Source
{
  File::Path file;
  bool loop = false;
  map<Speaker, vector<sample_t>> waveforms;
  double base_frequency = 0.0;
  double frequency = 0.0;

  //------------------------------------------------------------------------
  // Load a WAV into waveforms
  bool load_wav(const File::Path& f, map<Speaker, vector<sample_t>>& w);

  // Source/Element virtuals
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override;
  void set_property(const string& property, const SetParams& sp) override;
  void tick(const TickData& td) override;

public:
  WavSource(const Dataflow::Module *module, const XML::Element& config):
    Source(module, config) {}
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

  if (!load_wav(file, waveforms))
    return;

  Log::Detail dlog;
  dlog << "File loaded: " << file << "' in Wav '" << id << "'\n";
}

//--------------------------------------------------------------------------
// Load a WAV into samples
bool WavSource::load_wav(const File::Path& f, map<Speaker, vector<sample_t>>& w)
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

  vector<sample_t> s(length / sizeof(sample_t));
  memcpy(&s[0], buffer, length);
  SDL_FreeWAV(buffer);

  const auto cmit = channel_mappings.find(spec.channels);
  if (cmit == channel_mappings.end())
  {
    Log::Error log;
    log << "File with " << spec.channels << " channels not supported\n";
    return false;
  }
  const auto& channel_map = cmit->second;

  SDL_AudioCVT cvt;
  if (SDL_BuildAudioCVT(&cvt, spec.format, spec.channels, spec.freq,
                              AUDIO_F32, spec.channels, sample_rate) < 0)
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

  for (auto c = 0u; c < spec.channels; ++c)
  {
    auto& wc = w[channel_map.at(c)];
    wc.reserve(s.size() / spec.channels);
    for (auto i = 0u; i < s.size(); i += spec.channels)
      wc.push_back(s[i + c]);
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
  if (waveforms.empty())
    return;

  const auto step = frequency ? (frequency / base_frequency) : 1.0;
  const auto spos = step * td.sample_pos(sample_rate);

  const auto nsamples = td.samples(sample_rate);
  auto fragment = new Fragment(td.t);
  for (auto& cit: waveforms)
  {
    const auto& samples = cit.second;
    auto& waveform = fragment->waveforms[cit.first];
    waveform.reserve(nsamples);

    auto pos = spos;
    for (auto i = 0u; i < nsamples; ++i, pos += step)
    {
      auto p = pos;
      if (loop && p >= samples.size())
        p = fmod(p, samples.size());

      if (p >= samples.size())
      {
        waveform.push_back(0.0); // Pad with nothing
        continue;
      }

      auto fs = samples[p];
      auto cs = p + 1 > samples.size()
                ? (loop ? samples[0] : 0.0)
                : samples[p + 1];
      auto is = fs + ((cs - fs) * fmod(p, 1));
      waveform.push_back(is);
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
