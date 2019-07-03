//==========================================================================
// ViGraph dataflow module:
//    audio/sinks/sdl/audio-sink-sdl.cc
//
// Sink to output audio to the SDL output
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"
#include <SDL.h>

namespace {

using namespace ViGraph::Dataflow;

const auto default_device{"default"};
const auto default_channels = 2;
const auto default_buffer_size = 4096;

//==========================================================================
// SDL sink
class SDLSink: public FragmentSink
{
public:
  string device = default_device;
  int nchannels = default_channels;
  int buffer_size = default_buffer_size;
  bool starved = false;

private:
  SDL_AudioDeviceID dev = 0;
  vector<sample_t> output_buffer;

  // Source/Element virtuals
  void setup() override;
  void accept(FragmentPtr fragment) override;
  void shutdown() override;

public:
  using FragmentSink::FragmentSink;

  // Callback for SDL
  void callback(Uint8 *stream, int len);
};

//--------------------------------------------------------------------------
// Callback function that request data from us
void callback(void *userdata, Uint8 *stream, int len)
{
  auto sink = reinterpret_cast<SDLSink *>(userdata);
  if (sink)
    sink->callback(stream, len);
}

//--------------------------------------------------------------------------
// Callback for SDL
void SDLSink::callback(Uint8 *stream, int len)
{
  if (!dev)
    return;
  if (output_buffer.size() * sizeof(sample_t) >=
      (starved ? 1.5 : 1.0) * static_cast<unsigned>(len))
  {
    starved = false;
    copy(&output_buffer[0], &output_buffer[len / sizeof(sample_t)],
         reinterpret_cast<sample_t *>(stream));
    output_buffer.erase(output_buffer.begin(),
                        output_buffer.begin() + len / sizeof(sample_t));
  }
  else if (starved)
  {
    // We're on the edge so allow some slack to build up
    fill(&stream[0], &stream[len], 0.0);
  }
  else
  {
    copy(output_buffer.begin(), output_buffer.end(),
         reinterpret_cast<sample_t *>(stream));
    auto done = output_buffer.size() * sizeof(sample_t);
    output_buffer.clear();
    fill(&stream[done], &stream[len], 0.0);
    starved = true;
  }
}

//--------------------------------------------------------------------------
// Setup
void SDLSink::setup()
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

  if (SDL_InitSubSystem(SDL_INIT_AUDIO))
  {
    log.error << "Failed to start SDL audio subsystem: "
              << SDL_GetError() << endl;
    return;
  }

  log.summary << "Opening audio output on SDL device '" << device << "'\n";
  log.detail << "SDL: " << nchannels << " channels\n";

  try
  {
    SDL_AudioSpec want;
    SDL_AudioSpec have;

    SDL_memset(&want, 0, sizeof(want));
    want.freq = sample_rate;
    want.format = AUDIO_F32SYS;
    want.channels = nchannels;
    want.samples = buffer_size;
    want.userdata = this;
    want.callback = ::callback;

    // Open audio device
    dev = SDL_OpenAudioDevice(device == default_device
                              ? nullptr : device.c_str(),
                              0, &want, &have, 0);
    if (!dev)
      throw runtime_error(string("open: ") + SDL_GetError());

    // Start playback
    SDL_PauseAudioDevice(dev, 0);

    log.detail << "Created SDL audio out\n";
  }
  catch (const runtime_error& e)
  {
    log.error << "Can't open SDL audio output: " << e.what() << endl;
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
  }
}

//--------------------------------------------------------------------------
// Process some data
void SDLSink::accept(FragmentPtr fragment)
{
  auto& waveforms = fragment->waveforms;

  // Find max samples per waveform
  auto num_samples = vector<sample_t>::size_type{};
  for (auto& waveform: waveforms)
    num_samples = max(waveform.second.size(), num_samples);

  if (dev)
  {
    SDL_LockAudioDevice(dev);
    // Build an interleaved output buffer
    auto bstart = output_buffer.size();
    output_buffer.resize(bstart + num_samples * nchannels);
    auto wit = waveforms.begin();
    for (auto i = 0; i < nchannels; ++i)
    {
      if (wit == waveforms.end())
        break;
      for (auto s = 0ul; s < num_samples; ++s)
      {
        if (s >= wit->second.size())
          break;
        output_buffer[bstart + s * nchannels + i] = wit->second[s];
      }
      ++wit;
    }
    SDL_UnlockAudioDevice(dev);
  }
}

//--------------------------------------------------------------------------
// Shut down
void SDLSink::shutdown()
{
  if (dev) SDL_CloseAudioDevice(dev);
  dev = 0;
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "sdl-out",
  "SDL Audio output",
  "Audio output for SDL",
  "audio",
  {
      { "device", { "Device output to", Value::Type::text,
                    &SDLSink::device, false } },
      { "channels", { "Number of channels", Value::Type::number,
                      &SDLSink::nchannels, false } },
      { "buffer-size", { "Buffer size in samples (must be a power of 2)",
                         Value::Type::number,
                         &SDLSink::buffer_size, false } },
  },
  { "Audio" }, // inputs
  {}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SDLSink, module)

