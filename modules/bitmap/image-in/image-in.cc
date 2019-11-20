//==========================================================================
// ViGraph dataflow module: bitmap/image-in/image-in.cc
//
// Ingest bitmap data from a wav file
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"
#include <SDL.h>
#include <SDL_image.h>

namespace {

using namespace ViGraph::Dataflow;

//==========================================================================
// ImageIn
class ImageIn: public SimpleElement
{
private:
  Bitmap::Rectangle image_rect;

  // SimpleElement virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;

  // Clone
  ImageIn *create_clone() const override
  {
    return new ImageIn{module};
  }

public:
  ImageIn(const SimpleModule& module);

  // Settings
  Setting<string> file{};

  // Output
  Output<Bitmap::Group> output;
};

//--------------------------------------------------------------------------
// Constructor
ImageIn::ImageIn(const SimpleModule& module):
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

  SDL_IMAGE_VERSION(&compiled);
  linked = *IMG_Linked_Version();

  if (compiled.major != linked.major ||
      compiled.minor != linked.minor ||
      compiled.patch != linked.patch)
  {
    log.summary << "SDL_image compiled version: "
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
void ImageIn::setup(const SetupContext&)
{
  Log::Streams log;
  auto got = IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
  if (!(got & IMG_INIT_JPG)) log.error << "SDL_image missing JPEG support\n";
  if (!(got & IMG_INIT_PNG)) log.error << "SDL_image missing PNG support\n";

  auto surface_arb = IMG_Load(file.get().c_str());
  if (!surface_arb)
  {
    log.error << "Can't read image file " << file << ": " << IMG_GetError()
              << endl;
    return;
  }

  // Force 32-bit RGBA
  auto surface = SDL_ConvertSurfaceFormat(surface_arb,SDL_PIXELFORMAT_RGBA32,0);
  SDL_FreeSurface(surface_arb);

  SDL_LockSurface(surface);
  auto pixels = static_cast<Uint32 *>(surface->pixels);
  if (!pixels) return;

  log.detail << "Loaded " << surface->w << "x" << surface->h
             << " image from " << file << endl;

  image_rect = Bitmap::Rectangle(surface->w, surface->h);
  auto bytes_pp = surface->format->BytesPerPixel;
  for(auto y=0; y<surface->h; y++)
  {
    for(auto x=0; x<surface->w; x++)
    {
      // Note pixels is already 32-bit pointer
      auto pixel = pixels[(y*surface->pitch + x*bytes_pp)/4];
      Uint8 r,g,b,a;
      SDL_GetRGBA(pixel, surface->format, &r, &g, &b, &a);
      image_rect.set(x, y, Colour::RGBA(r/255.0, g/255.0, b/255.0, a/255.0));
    }
  }

  SDL_UnlockSurface(surface);
  SDL_FreeSurface(surface);
  IMG_Quit();
}

//--------------------------------------------------------------------------
// Process some data
void ImageIn::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(nsamples, {}, {}, tie(output),
                 [&](Bitmap::Group& output)
  {
    output.add(image_rect);
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::DynamicModule module
{
  "image-in",
  "Image file input",
  "bitmap",
  {
    { "file",   &ImageIn::file   }
  },
  {},
  {
    { "output", &ImageIn::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ImageIn, module)
