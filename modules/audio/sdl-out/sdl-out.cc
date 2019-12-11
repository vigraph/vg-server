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
class SDLSink: public DynamicElement
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
  SDLSink(const DynamicModule& module);

  Setting<string> device{default_device};
  Setting<Number> nchannels{default_channels};
  Setting<Number> buffer_size{default_buffer_size};
  Setting<Number> max_delay{default_max_delay};
  Setting<Number> max_recovery{default_max_recovery};

  Input<Number> channel1;
  Input<Number> channel2;
  Input<Number> channel3;
  Input<Number> channel4;
  Input<Number> channel5;
  Input<Number> channel6;

  // Callback for SDL
  void callback(Uint8 *stream, int len);

  ~SDLSink();
};

//--------------------------------------------------------------------------
// Constructor
SDLSink::SDLSink(const DynamicModule& module):
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
void SDLSink::setup(const SetupContext&)
{
  Log::Streams log;
  shutdown();

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
                              ? nullptr : device.get().c_str(),
                              0, &want, &have, 0);
    if (!dev)
      throw runtime_error(string("open: ") + SDL_GetError());

    // Start playback
    SDL_PauseAudioDevice(dev, 0);

    // Update module information
    if (have.channels < 6 && module.num_inputs() >= 6)
      module.erase_input("channel6");
    if (have.channels < 5 && module.num_inputs() >= 5)
      module.erase_input("channel5");
    if (have.channels < 4 && module.num_inputs() >= 4)
      module.erase_input("channel4");
    if (have.channels < 3 && module.num_inputs() >= 3)
      module.erase_input("channel3");
    if (have.channels < 2 && module.num_inputs() >= 2)
      module.erase_input("channel2");
    if (have.channels < 1 && module.num_inputs() >= 1)
      module.erase_input("channel1");
    if (have.channels >= 1 && module.num_inputs() < 1)
      module.add_input("channel1", &SDLSink::channel1);
    if (have.channels >= 2 && module.num_inputs() < 2)
      module.add_input("channel2", &SDLSink::channel2);
    if (have.channels >= 3 && module.num_inputs() < 3)
      module.add_input("channel3", &SDLSink::channel3);
    if (have.channels >= 4 && module.num_inputs() < 4)
      module.add_input("channel4", &SDLSink::channel4);
    if (have.channels >= 5 && module.num_inputs() < 5)
      module.add_input("channel5", &SDLSink::channel5);
    if (have.channels >= 6 && module.num_inputs() < 6)
      module.add_input("channel6", &SDLSink::channel6);

    channel1.set_sample_rate(have.freq);
    channel2.set_sample_rate(have.freq);
    channel3.set_sample_rate(have.freq);
    channel4.set_sample_rate(have.freq);
    channel5.set_sample_rate(have.freq);
    channel6.set_sample_rate(have.freq);

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
    const auto nsamples = td.samples_in_tick(channel1.get_sample_rate());
    auto bpos = output_buffer.size();
    const auto channels = nchannels;
    output_buffer.resize(bpos + nsamples * channels);
    sample_iterate(td, nsamples, {},
                   tie(channel1, channel2, channel3,
                       channel4, channel5, channel6), {},
                   [&](Number c1, Number c2, Number c3,
                       Number c4, Number c5, Number c6)
    {
      output_buffer[bpos++] = c1;
      if (channels >= 2)
        output_buffer[bpos++] = c2;
      if (channels >= 3)
        output_buffer[bpos++] = c3;
      if (channels >= 4)
        output_buffer[bpos++] = c4;
      if (channels >= 5)
        output_buffer[bpos++] = c5;
      if (channels >= 6)
        output_buffer[bpos++] = c6;
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
Dataflow::DynamicModule module
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
    // dynamic input channels
  },
  {}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SDLSink, module)

