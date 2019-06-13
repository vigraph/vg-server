//==========================================================================
// ViGraph dataflow module:
//    audio/sources/sdl/audio-source-sdl.cc
//
// Source to capture audio from SDL
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"
#include <SDL.h>

namespace {

using namespace ViGraph::Dataflow;
using namespace ViGraph::Module::Audio;

const auto default_device{"default"};
const auto default_channels = 1;

//==========================================================================
// SDL source
class SDLSource: public FragmentSource
{
public:
  string device = default_device;
  int nchannels = default_channels;

private:
  SDL_AudioDeviceID dev = 0;
  vector<sample_t> input_buffer;
  vector<Speaker> channel_mapping;

  // Source/Element virtuals
  void setup() override;
  void tick(const TickData& td) override;
  void shutdown() override;

public:
  using FragmentSource::FragmentSource;

  // Callback for SDL
  void callback(Uint8 *stream, int len);
};

//--------------------------------------------------------------------------
// Callback function that request data from us
void callback(void *userdata, Uint8 *stream, int len)
{
  auto source = reinterpret_cast<SDLSource *>(userdata);
  if (source)
    source->callback(stream, len);
}

//--------------------------------------------------------------------------
// Callback for SDL
void SDLSource::callback(Uint8 *stream, int len)
{
  auto p = reinterpret_cast<sample_t *>(stream);
  input_buffer.insert(input_buffer.end(), p, p + (len / sizeof(sample_t)));
}

//--------------------------------------------------------------------------
// Setup
void SDLSource::setup()
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

  try
  {
    log.summary << "Opening audio capture on SDL device '" << device << "'\n";

    const auto cmit = channel_mappings.find(nchannels);
    if (cmit == channel_mappings.end())
      throw runtime_error(string("Unsupported channel count: ") +
                          Text::itos(nchannels));
    channel_mapping = cmit->second;

    log.detail << "SDL: " << nchannels << " channels\n";

    SDL_AudioSpec want;
    SDL_AudioSpec have;

    SDL_memset(&want, 0, sizeof(want));
    want.freq = sample_rate;
    want.format = AUDIO_F32SYS;
    want.channels = nchannels;
    want.samples = 4096;
    want.userdata = this;
    want.callback = ::callback;

    // Open audio device
    dev = SDL_OpenAudioDevice(device == default_device
                              ? nullptr : device.c_str(),
                              1, &want, &have, 0);
    if (!dev)
      throw runtime_error(string("open: ") + SDL_GetError());

    // Start capture
    SDL_PauseAudioDevice(dev, 0);

    log.detail << "Created SDL audio capture\n";
  }
  catch (const runtime_error& e)
  {
    log.error << "Can't open SDL audio capture: " << e.what() << endl;
  }
}

//--------------------------------------------------------------------------
// Read some data
void SDLSource::tick(const TickData& td)
{
  if (dev)
  {
    const auto nsamples = td.samples();
    auto fragment = new Fragment(td.t);
    const auto nchannels = channel_mapping.size();
    SDL_LockAudioDevice(dev);
    for (auto c = 0u; c < nchannels; ++c)
    {
      auto& wc = fragment->waveforms[channel_mapping[c]];
      wc.resize(nsamples);
      for (auto i = 0u; i < nsamples && i < (input_buffer.size() / nchannels);
           ++i)
        wc[i] = input_buffer[i * nchannels + c];
    }
    if (nsamples * nchannels >= input_buffer.size())
      input_buffer.clear();
    else
      input_buffer.erase(input_buffer.begin(),
                         input_buffer.begin() + nsamples * nchannels);
    SDL_UnlockAudioDevice(dev);
    send(fragment);
  }
}

//--------------------------------------------------------------------------
// Shut down
void SDLSource::shutdown()
{
  if (dev) SDL_CloseAudioDevice(dev);
  dev = 0;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "sdl-in",
  "SDL Audio input",
  "Audio input for SDL",
  "audio",
  {
      { "device",  { "Device to capture from", Value::Type::text,
                     &SDLSource::device, false } },
      { "channels", { "Number of channels", Value::Type::number,
                      &SDLSource::nchannels, false } },
  },
  {}, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SDLSource, module)
