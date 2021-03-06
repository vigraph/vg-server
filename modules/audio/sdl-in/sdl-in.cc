//==========================================================================
// ViGraph dataflow module: audio/sdl-out/sdl-out.cc
//
// Input audio from SDL
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../audio-module.h"
#include <SDL.h>

namespace {

using namespace ViGraph::Dataflow;

const auto default_device{"default"};
const auto default_sample_rate{44100};
const auto default_channels = 1;

//==========================================================================
// SDL out
class SDLIn: public SimpleElement
{
private:
  SDL_AudioDeviceID dev = 0;
  bool sdl_inited = false;
  vector<float> input_buffer;
  double input_sample_rate = default_sample_rate;
  double pos = 0.0;

  // Source/Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;
  void shutdown();

  // Clone
  SDLIn *create_clone() const override
  {
    return new SDLIn{module};
  }

public:
  SDLIn(const SimpleModule& module);

  Setting<string> device{default_device};
  Setting<Number> sample_rate{default_sample_rate};
  Setting<Number> nchannels{default_channels};

  Output<AudioData> output;

  // Callback for SDL
  void callback(Uint8 *stream, int len);

  ~SDLIn();
};

//--------------------------------------------------------------------------
// Constructor
SDLIn::SDLIn(const SimpleModule& module):
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
  auto source = reinterpret_cast<SDLIn *>(userdata);
  if (source)
    source->callback(stream, len);
}

//--------------------------------------------------------------------------
// Callback for SDL
void SDLIn::callback(Uint8 *stream, int len)
{
  auto p = reinterpret_cast<sample_t *>(stream);
  input_buffer.insert(input_buffer.end(), p, p + (len / sizeof(sample_t)));
}

//--------------------------------------------------------------------------
// Setup
void SDLIn::setup(const SetupContext& context)
{
  SimpleElement::setup(context);

  Log::Streams log;
  shutdown();

  log.summary << "Opening audio input on SDL device '" << device << "'\n";

  try
  {
    log.summary << "Opening audio capture on SDL device '" << device << "'\n";

    if (nchannels > max_channels)
      throw runtime_error("Too many channels in SDL input");

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
    const auto& dname = device.get();
    dev = SDL_OpenAudioDevice(dname == default_device
                              ? nullptr : dname.c_str(),
                              1, &want, &have, 0);
    if (!dev)
      throw runtime_error(string("open: ") + SDL_GetError());

    input_sample_rate = have.freq;

    log.detail << "SDL: " << have.channels << " channels\n";

    // Start capture
    SDL_PauseAudioDevice(dev, 0);

    log.detail << "Created SDL audio capture\n";
  }
  catch (const runtime_error& e)
  {
    log.error << "Can't open SDL audio input: " << e.what() << endl;
  }
}

//--------------------------------------------------------------------------
// Process some data
void SDLIn::tick(const TickData& td)
{
  if (dev)
  {
    pos = fmod(pos, 1);
    auto sample_rate = output.get_sample_rate();
    auto buffer = output.get_buffer(td);
    const auto nsamples = td.samples_in_tick(sample_rate);
    const auto step = input_sample_rate / sample_rate;
    const auto channels = nchannels.get();

    SDL_LockAudioDevice(dev);
    const auto available = input_buffer.size() / channels;
    for (auto i = 0u; i < nsamples; ++i)
    {
      const auto p = fmod(pos, 1);
      const auto i1 = static_cast<unsigned>(pos);
      const auto i2 = i1 + 1;

      buffer.data.push_back(AudioData());
      auto& ad = buffer.data.back();
      ad.nchannels = channels;
      for(auto c=0; c<channels; c++)
      {
        const auto c1 = i1 * channels + c;
        const auto c2 = i2 * channels + c;
        const auto s1 = c1 < available ? input_buffer[c1] : 0.0;
        const auto s2 = c2 < available ? input_buffer[c2] : s1;
        ad.channels[c] = s1 + (s2 - s1) * p;
      }
      pos += step;
    }
    input_buffer.erase(input_buffer.begin(),
                       input_buffer.begin() +
                       min(available, channels * (pos - step)));
    SDL_UnlockAudioDevice(dev);
  }
}

//--------------------------------------------------------------------------
// Shut down
void SDLIn::shutdown()
{
  if (dev) SDL_CloseAudioDevice(dev);
  dev = 0;
}

//--------------------------------------------------------------------------
// Destructor
SDLIn::~SDLIn()
{
  shutdown();
  if (sdl_inited)
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
  sdl_inited = false;
}

//--------------------------------------------------------------------------
// Module definition
SimpleModule module
{
  "sdl-in",
  "SDL Audio input",
  "audio",
  {
    { "device", &SDLIn::device },
    { "sample-rate", &SDLIn::sample_rate },
    { "channels", &SDLIn::nchannels },
  },
  {},
  {
    { "output", &SDLIn::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SDLIn, module)
