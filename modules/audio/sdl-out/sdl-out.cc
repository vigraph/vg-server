//==========================================================================
// ViGraph dataflow module: audio/sdl-out/sdl-out.cc
//
// Output audio to the SDL output
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../audio-module.h"
#include <SDL.h>

namespace {

using namespace ViGraph::Dataflow;

const auto default_device{"default"};
const auto default_channels = 2;
const auto default_buffer_size = 4096;
const auto default_max_delay = 4000;
const auto default_max_recovery = 1;

//==========================================================================
// SDL sink
class SDLSink: public SimpleElement
{
private:
  SDL_AudioDeviceID dev = 0;
  bool sdl_inited = false;
  vector<sample_t> output_buffer;
  bool starved = false;

  // Source/Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;
  void shutdown() override;

  // Clone
  SDLSink *create_clone() const override
  {
    return new SDLSink{module};
  }

public:
  SDLSink(const SimpleModule& module);

  Setting<string> device{default_device};
  Setting<Number> nchannels{default_channels};
  Setting<Number> buffer_size{default_buffer_size};
  Setting<Number> max_delay{default_max_delay};
  Setting<Number> max_recovery{default_max_recovery};

  Input<AudioData> input;

  // Callback for SDL
  void callback(Uint8 *stream, int len);

  ~SDLSink();
};

//--------------------------------------------------------------------------
// Constructor
SDLSink::SDLSink(const SimpleModule& module):
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

  if (SDL_InitSubSystem(SDL_INIT_AUDIO))
  {
    log.error << "Failed to start SDL audio subsystem: "
              << SDL_GetError() << endl;
    return;
  }
  sdl_inited = true;
}

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
    copy(&output_buffer[0], &output_buffer[0] + len / sizeof(sample_t),
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
    auto have = output_buffer.size() * sizeof(sample_t);
    auto hstart = len - have;
    fill(&stream[0], &stream[0] + hstart, 0);
    copy(output_buffer.begin(), output_buffer.end(),
         reinterpret_cast<sample_t *>(stream + hstart));
    output_buffer.clear();
    starved = true;
  }
}

//--------------------------------------------------------------------------
// Setup
void SDLSink::setup(const SetupContext& context)
{
  SimpleElement::setup(context);

  Log::Streams log;
  shutdown();

  log.summary << "Opening audio output on SDL device '" << device << "'\n";
  if (nchannels > max_channels)
    throw runtime_error("Too many channels in SDL output");
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
                              ? nullptr : device.get().c_str(),
                              0, &want, &have, 0);
    if (!dev)
      throw runtime_error(string("open: ") + SDL_GetError());

    // Start playback
    SDL_PauseAudioDevice(dev, 0);

    input.set_sample_rate(have.freq);

    log.detail << "Created SDL audio out\n";
  }
  catch (const runtime_error& e)
  {
    log.error << "Can't open SDL audio output: " << e.what() << endl;
  }
}

//--------------------------------------------------------------------------
// Process some data
void SDLSink::tick(const TickData& td)
{
  if (dev)
  {
    SDL_LockAudioDevice(dev);
    const auto nsamples = td.samples_in_tick(input.get_sample_rate());
    auto bpos = output_buffer.size();
    const auto channels = nchannels;
    output_buffer.resize(bpos + nsamples * channels);
    sample_iterate(td, nsamples, {},
                   tie(input), {},
                   [&](const AudioData& input)
    {
      for(auto c=0; c<input.nchannels; c++)
        output_buffer[bpos++] = input.channels[c];

      // Zero any we don't get
      for(auto c=input.nchannels; c<channels; c++)
        output_buffer[bpos++] = 0;
    });
    SDL_UnlockAudioDevice(dev);
  }
}

//--------------------------------------------------------------------------
// Shut down
void SDLSink::shutdown()
{
  if (dev) SDL_CloseAudioDevice(dev);
  dev = 0;
}

//--------------------------------------------------------------------------
// Destructor
SDLSink::~SDLSink()
{
  if (sdl_inited)
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
  sdl_inited = false;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "sdl-out",
  "SDL Audio output",
  "audio",
  {
    { "device", &SDLSink::device },
    { "channels", &SDLSink::nchannels },
    { "buffer-size", &SDLSink::buffer_size },
    { "max-delay", &SDLSink::max_delay },
    { "max-recovery", &SDLSink::max_recovery },
  },
  {
    { "input", &SDLSink::input }
  },
  {}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SDLSink, module)

