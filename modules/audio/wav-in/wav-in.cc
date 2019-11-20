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
class WavIn: public DynamicElement
{
private:
  vector<vector<double>> waveforms;
  string wav_file;
  double wav_sample_rate = 0;
  unsigned wav_nsamples = 0;
  double pos = 0;
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
  WavIn(const DynamicModule& module);

  Setting<string> file{};
  Setting<bool> loop{false};

  Input<double> start{0.0};
  Input<double> stop{0.0};

  Output<double> channel1;
  Output<double> channel2;
  Output<double> channel3;
  Output<double> channel4;
  Output<double> channel5;
  Output<double> channel6;
};

//--------------------------------------------------------------------------
// Constructor
WavIn::WavIn(const DynamicModule& module):
  DynamicElement(module)
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
  Log::Streams log;

  if (wav_file == file.get())
    return;

  waveforms.clear();
  wav_nsamples = 0;
  const auto f = context.get_file_path(file);

  if (!f.exists())
  {
    Log::Error log;
    log << "File not found: '" << f << "' in WavIn '" << get_id() << "'\n";
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

  // Update module information
  if (waveforms.size() < 6 && module.num_outputs() >= 6)
    module.erase_output("channel6");
  if (waveforms.size() < 5 && module.num_outputs() >= 5)
    module.erase_output("channel5");
  if (waveforms.size() < 4 && module.num_outputs() >= 4)
    module.erase_output("channel4");
  if (waveforms.size() < 3 && module.num_outputs() >= 3)
    module.erase_output("channel3");
  if (waveforms.size() < 2 && module.num_outputs() >= 2)
    module.erase_output("channel2");
  if (waveforms.size() < 1 && module.num_outputs() >= 1)
    module.erase_output("channel1");
  if (waveforms.size() >= 1 && module.num_outputs() < 1)
    module.add_output("channel1", &WavIn::channel1);
  if (waveforms.size() >= 2 && module.num_outputs() < 2)
    module.add_output("channel2", &WavIn::channel2);
  if (waveforms.size() >= 3 && module.num_outputs() < 3)
    module.add_output("channel3", &WavIn::channel3);
  if (waveforms.size() >= 4 && module.num_outputs() < 4)
    module.add_output("channel4", &WavIn::channel4);
  if (waveforms.size() >= 5 && module.num_outputs() < 5)
    module.add_output("channel5", &WavIn::channel5);
  if (waveforms.size() >= 6 && module.num_outputs() < 6)
    module.add_output("channel6", &WavIn::channel6);

  log.detail << "Loaded wav file '" << f << "' with " << waveforms.size()
             << " channels\n";
}

//--------------------------------------------------------------------------
// Process some data
void WavIn::tick(const TickData& td)
{
  const auto sample_rate = max(channel1.get_sample_rate(),
                               max(channel2.get_sample_rate(),
                                   max(channel3.get_sample_rate(),
                                       max(channel4.get_sample_rate(),
                                           max(channel5.get_sample_rate(),
                                               channel6.get_sample_rate())))));
  const auto nsamples = td.samples_in_tick(sample_rate);
  const auto step = wav_sample_rate / sample_rate;
  const auto channels = min(waveforms.size(), 6ul);
  sample_iterate(nsamples, {},
                 tie(start, stop),
                 tie(channel1, channel2, channel3,
                     channel4, channel5, channel6),
                 [&](double _start, double _stop,
                     double& c1, double& c2, double& c3,
                     double& c4, double& c5, double& c6)
  {
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

    switch (state)
    {
      case State::enabled:
      case State::completing:
        {
          const auto p = fmod(pos, 1);
          const auto i1 = static_cast<unsigned>(pos);
          const auto i2 = (i1 + 1 >= wav_nsamples) ? 0 : i1 + 1;
          if (channels >= 1)
            c1 = waveforms[0][i1] + ((waveforms[0][i2] - waveforms[0][i1]) * p);
          if (channels >= 2)
            c2 = waveforms[1][i1] + ((waveforms[1][i2] - waveforms[1][i1]) * p);
          if (channels >= 3)
            c3 = waveforms[2][i1] + ((waveforms[2][i2] - waveforms[2][i1]) * p);
          if (channels >= 4)
            c4 = waveforms[3][i1] + ((waveforms[3][i2] - waveforms[3][i1]) * p);
          if (channels >= 5)
            c5 = waveforms[4][i1] + ((waveforms[4][i2] - waveforms[4][i1]) * p);
          if (channels >= 6)
            c6 = waveforms[5][i1] + ((waveforms[5][i2] - waveforms[5][i1]) * p);
          pos += step;
          if (pos >= wav_nsamples)
          {
            pos = fmod(pos, wav_nsamples);
            if (!loop || state == State::completing)
              state = State::complete;
          }
        }
        break;
      case State::disabled:
      case State::complete:
        if (channels >= 1)
          c1 = 0;
        if (channels >= 2)
          c2 = 0;
        if (channels >= 3)
          c3 = 0;
        if (channels >= 4)
          c4 = 0;
        if (channels >= 5)
          c5 = 0;
        if (channels >= 6)
          c6 = 0;
        break;
    }
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::DynamicModule module
{
  "wav-in",
  "Wav file input",
  "audio",
  {
    { "file", &WavIn::file },
    { "loop", &WavIn::loop },
  },
  {
    // dynamic input channels
  },
  {}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(WavIn, module)
