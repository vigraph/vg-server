//==========================================================================
// ViGraph dataflow module: bitmap/sdl-out/sdl-out.cc
//
// Output bitmap to the SDL output
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"
#include <SDL.h>

namespace {

using namespace ViGraph::Dataflow;

const auto default_frame_rate = 25;
const auto default_width = 640;
const auto default_height = 480;

//==========================================================================
// SDL sink
class SDLWindow: public SimpleElement
{
private:
  bool sdl_inited = false;
  SDL_Window *window{0};
  SDL_Renderer *renderer{0};
  SDL_Texture *texture{0};

  // Source/Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;
  void shutdown() override;

  // Clone
  SDLWindow *create_clone() const override
  {
    return new SDLWindow{module};
  }

public:
  SDLWindow(const SimpleModule& module);

  Setting<Number> frame_rate{default_frame_rate};
  Setting<Integer> width{default_width};
  Setting<Integer> height{default_height};
  Setting<bool> full_screen{false};

  Input<Bitmap::Group> input;

  ~SDLWindow();
};

//--------------------------------------------------------------------------
// Constructor
SDLWindow::SDLWindow(const SimpleModule& module):
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

  if (SDL_InitSubSystem(SDL_INIT_VIDEO))
  {
    log.error << "Failed to start SDL video subsystem: "
              << SDL_GetError() << endl;
    return;
  }
  sdl_inited = true;
}

//--------------------------------------------------------------------------
// Setup
void SDLWindow::setup(const SetupContext& context)
{
  SimpleElement::setup(context);

  shutdown();

  Log::Streams log;
  log.summary << "Opening SDL window " << width << "x" << height << endl;

  try
  {
    window = SDL_CreateWindow(
        "ViGraph bitmap window",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width, height,
        full_screen?SDL_WINDOW_FULLSCREEN:SDL_WINDOW_RESIZABLE);
    if (!window) throw runtime_error(string("window: ")+SDL_GetError());

    SDL_RaiseWindow(window);

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) throw runtime_error(string("renderer: ")+SDL_GetError());

    texture = SDL_CreateTexture(renderer,
                                SDL_PIXELFORMAT_ABGR8888,
                                SDL_TEXTUREACCESS_STREAMING,
                                width, height);
    if (!texture) throw runtime_error(string("texture: ")+SDL_GetError());

    log.detail << "Created SDL bitmap out\n";
    input.set_sample_rate(frame_rate);
  }
  catch (const runtime_error& e)
  {
    log.error << "Can't open SDL bitmap output: " << e.what() << endl;
  }
}

//--------------------------------------------------------------------------
// Process some data
void SDLWindow::tick(const TickData& td)
{
  const auto sample_rate = input.get_sample_rate();
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, tie(input), {},
                 [&](const Bitmap::Group& input)
  {
    Bitmap::Rectangle frame(width, height);
    frame.fill(Colour::black);
    input.compose(frame);

    void *pixels;
    int pitch;
    if (SDL_LockTexture(texture, NULL, &pixels, &pitch))
    {
      Log::Error log;
      log << "Can't lock SDL texture\n";
      return;
    }

    if (pitch == width * 4)
    {
      uint32_t *wpixels = reinterpret_cast<uint32_t *>(pixels);
      const auto& ipixels = frame.get_pixels();
      for(const auto& p: ipixels)
        *wpixels++ = p.packed;
    }
    else
    {
      Log::Error log;
      log << "Unexpected SDL texture pitch: " << pitch << endl;
    }

    SDL_UnlockTexture(texture);

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
  });
}

//--------------------------------------------------------------------------
// Shut down
void SDLWindow::shutdown()
{
  if (texture) SDL_DestroyTexture(texture);
  texture = 0;
  if (renderer) SDL_DestroyRenderer(renderer);
  renderer = 0;
  if (window) SDL_DestroyWindow(window);
  window = 0;
}

//--------------------------------------------------------------------------
// Destructor
SDLWindow::~SDLWindow()
{
  if (sdl_inited)
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
  sdl_inited = false;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "sdl-out",
  "SDL bitmap window",
  "bitmap",
  {
    { "width",  &SDLWindow::width },
    { "height", &SDLWindow::height },
    { "frame-rate",  &SDLWindow::frame_rate },
    { "full-screen",  &SDLWindow::full_screen }
  },
  {
    { "input", &SDLWindow::input }
  },
  {}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SDLWindow, module)

