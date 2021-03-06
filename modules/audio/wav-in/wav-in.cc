//==========================================================================
// ViGraph dataflow module: audio/wav-in/wav-in.cc
//
// Ingest audio data from a wav file
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../audio-module.h"
#include <SDL.h>

namespace {

using namespace ViGraph::Dataflow;

//==========================================================================
// WavIn
class WavIn: public SimpleElement
{
private:
  vector<vector<Number>> waveforms;
  string wav_file;
  Number wav_sample_rate = 0;
  unsigned wav_nsamples = 0;
  Number pos = 0;
  enum class State
  {
    disabled,
    enabled,
    completing,
    complete
  } state = State::disabled;

  // Source/Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;

  // Clone
  WavIn *create_clone() const override
  {
    return new WavIn{module};
  }

public:
  WavIn(const SimpleModule& module);

  Setting<string> file{};
  Setting<bool> loop{false};

  Input<Trigger> start{0};
  Input<Trigger> stop{0};

  Output<AudioData> output;
  Output<Trigger> finished;
};

//--------------------------------------------------------------------------
// Constructor
WavIn::WavIn(const SimpleModule& module):
  SimpleElement(module)
{
  Log::Streams log;
  SDL_version linked;
  SDL_GetVersion(&linked);
  SDL_version compiled;
  SDL_VERSION(&compiled);
  if (compiled.major != linked.major ||
      compiled.minor != linked.minor ||
      compiled.patch != linked.patch)
  {
    log.summary << "SDL compiled version: "
                << static_cast<int>(compiled.major) << "."
                << static_cast<int>(compiled.minor) << "."
                << static_cast<int>(compiled.patch)
                << ", linked version: "
                << static_cast<int>(linked.major) << "."
                << static_cast<int>(linked.minor) << "."
                << static_cast<int>(linked.patch) << endl;
  }
}

//--------------------------------------------------------------------------
// Setup
void WavIn::setup(const SetupContext& context)
{
  SimpleElement::setup(context);

  Log::Streams log;

  if (wav_file == file.get())
    return;

  waveforms.clear();
  wav_nsamples = 0;
  const auto f = context.get_file_path(file);

  if (!f || !f.exists())
  {
    Log::Error log;
    log << "File not found: '" << file << "' in WavIn '" << get_id() << "'\n";
    return;
  }

  SDL_AudioSpec spec;
  Uint8 *buffer = nullptr;
  Uint32 length = 0;
  if (!SDL_LoadWAV(f.c_str(), &spec, &buffer, &length))
  {
    Log::Error log;
    log << "File cannot be loaded: '" << f << "': " << SDL_GetError()
        << " in WavIn '" << get_id() << "'\n";
    return;
  }

  if (!length)
  {
    Log::Error log;
    log << "Empty wav file: '" << f << "' in WavIn '" << get_id() << "'\n";
    return;
  }

  vector<float> s(length / sizeof(float));
  memcpy(&s[0], buffer, length);
  SDL_FreeWAV(buffer);

  SDL_AudioCVT cvt;
  if (SDL_BuildAudioCVT(&cvt, spec.format, spec.channels, spec.freq,
                              AUDIO_F32, spec.channels, spec.freq) < 0)
  {
    Log::Error log;
    log << "Cannot prepare file for format conversion: '" << f << "': "
        << SDL_GetError() << " in WavIn '" << get_id() << "'\n";
    s.clear();
    return;
  }

  if (cvt.needed)
  {
    const auto len_needed = length * cvt.len_mult;
    cvt.len = length;
    if (len_needed > length)
    {
      // We're going to need a bigger boat
      s.resize(len_needed / sizeof(float));
    }
    cvt.buf = reinterpret_cast<Uint8 *>(&s[0]);
    if (SDL_ConvertAudio(&cvt))
    {
      Log::Error log;
      log << "Could not convert file to usable format: '" << f << "': "
          << SDL_GetError() << " in WavIn '" << get_id() << "'\n";
      s.clear();
      return;
    }
    s.resize(cvt.len_ratio * cvt.len / sizeof(float));
  }

  waveforms.resize(spec.channels);
  wav_file = file;
  wav_sample_rate = spec.freq;
  for (auto c = 0u; c < spec.channels; ++c)
  {
    auto& wc = waveforms[c];
    wc.reserve(s.size() / spec.channels);
    for (auto i = 0u; i < s.size(); i += spec.channels)
      wc.push_back(s[i + c]);
    wav_nsamples = wc.size();
  }

  log.detail << "Loaded wav file '" << f << "' with " << waveforms.size()
             << " channels\n";
}

//--------------------------------------------------------------------------
// Process some data
void WavIn::tick(const TickData& td)
{
  auto sample_rate = output.get_sample_rate();
  const auto nsamples = td.samples_in_tick(sample_rate);
  const auto step = wav_sample_rate / sample_rate;
  const auto nchannels = min(waveforms.size(), max_channels);

  sample_iterate(td, nsamples, {}, tie(start, stop), tie(output, finished),
                 [&](Trigger _start, Trigger _stop, AudioData& ad, Trigger &f)
  {
    f = 0;
    if (_stop)
    {
      if (state == State::enabled)
        state = State::completing;
    }
    else if (_start || (!start.connected() && state == State::disabled))
    {
      state = State::enabled;
      pos = 0.0;
    }

    ad.nchannels = nchannels;

    switch (state)
    {
      case State::enabled:
      case State::completing:
        {
          const auto p = fmod(pos, 1);
          const auto i1 = static_cast<unsigned>(pos);
          const auto i2 = (i1 + 1 >= wav_nsamples) ? 0 : i1 + 1;

          for(auto c=0; c<ad.nchannels; c++)
          {
            const auto& waveform = waveforms[c];
            const auto s = waveform[i1] + ((waveform[i2] - waveform[i1]) * p);
            ad.channels[c] = s;
          }
          pos += step;
          if (pos >= wav_nsamples)
          {
            pos = fmod(pos, wav_nsamples);
            f = 1;
            if (!loop || state == State::completing)
              state = State::complete;
          }
        }
        break;

      case State::disabled:
      case State::complete:
        for(auto c=0; c<ad.nchannels; c++)
            ad.channels[c] = 0;
        break;
    }
  });
}

//--------------------------------------------------------------------------
// Module definition
SimpleModule module
{
  "wav-in",
  "Wav file input",
  "audio",
  {
    { "file",     &WavIn::file },
    { "loop",     &WavIn::loop },
  },
  {
    { "start",    &WavIn::start },
    { "stop",     &WavIn::stop  },
  },
  {
    { "finished", &WavIn::finished  },
    { "output",   &WavIn::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(WavIn, module)
