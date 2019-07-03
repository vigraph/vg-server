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
class WavSource: public FragmentSource
{
public:
  string file;
  bool loop = false;
  double base_frequency = 0.0;
  double frequency = 0.0;
  double pos = 0.0;

private:
  map<Speaker, vector<sample_t>> waveforms;
  enum State
  {
    disabled,
    enabled,
    completing
  } state = State::enabled;

  //------------------------------------------------------------------------
  // Load a WAV into waveforms
  bool load_wav(const File::Path& f, map<Speaker, vector<sample_t>>& w);

  // Source/Element virtuals
  void setup(const File::Directory& base_dir) override;
  void tick(const TickData& td) override;
  void notify_target_of(const string& property) override;

public:
  using FragmentSource::FragmentSource;

  void start()
  {
    if (state != State::enabled)
    {
      if (state != State::completing)
        pos = 0.0;
      state = State::enabled;
    }
    pos = 0;
  }
  void stop() { if (state == State::enabled) state = State::completing; }
};

//--------------------------------------------------------------------------
// Setup
void WavSource::setup(const File::Directory& base_dir)
{
  if (frequency && !base_frequency)
  {
    Log::Error log;
    log << "Frequency requires that base frequency is also set in Wav '"
        << id << "'\n";
    return;
  }

  if (!load_wav(base_dir.resolve(file), waveforms))
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
// If recipient of on/offs default to disabled
void WavSource::notify_target_of(const string& property)
{
  if (property == "trigger")
    state = State::disabled;
}

//--------------------------------------------------------------------------
// Generate a fragment
void WavSource::tick(const TickData& td)
{
  if (waveforms.empty())
    return;

  if (state == State::enabled || state == State::completing)
  {
    const auto step = frequency ? (frequency / base_frequency) : 1.0;
    const auto spos = pos;

    const auto nsamples = td.samples();
    auto fragment = new Fragment(td.t);
    bool complete = false;
    for (auto& cit: waveforms)
    {
      const auto& samples = cit.second;
      auto& waveform = fragment->waveforms[cit.first];
      waveform.reserve(nsamples);

      pos = spos;
      for (auto i = 0u; i < nsamples; ++i, pos += step)
      {
        auto p = pos;
        if (p >= samples.size())
        {
          if (!loop || state == State::completing)
          {
            waveform.push_back(0.0); // Pad with nothing
            complete = true;
            continue;
          }
          p = fmod(p, samples.size());
        }

        auto fs = samples[p];
        auto cs = p + 1 >= samples.size()
                  ? (loop ? samples[0] : 0.0)
                  : samples[p + 1];
        auto is = fs + ((cs - fs) * fmod(p, 1));
        waveform.push_back(is);
      }
    }

    if (complete)
    {
      state = State::disabled;
      pos = 0;
    }

    // Send to output
    send(fragment);
  }
}

Dataflow::Module module
{
  "wav",
  "Wav",
  "Audio Wav Input",
  "audio",
  {
    { "file",  { "File path", Value::Type::file,
                 &WavSource::file, false } },
    { "loop",  { "Loop", Value::Type::boolean,
                 &WavSource::loop, true } },
    { "base-freq", { "Base frequency", Value::Type::number,
                     &WavSource::base_frequency, false } },
    { "freq",  { "Frequency to play at", Value::Type::number,
                 &WavSource::frequency, true } },
    { "trigger", { "Trigger playing", Value::Type::trigger,
                   &WavSource::start, true } },
    { "clear", { "Stop playing at end of current loop", Value::Type::trigger,
                   &WavSource::stop, true } },
  },
  {},  // no inputs
  { "Audio" }  // outputs
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(WavSource, module)
