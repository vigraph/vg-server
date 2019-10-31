//==========================================================================
// ViGraph dataflow module: dmx/bitmap-render/bitmap-render.cc
//
// Render a bitmap into DMX light values
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../dmx-module.h"
#include "../../bitmap/bitmap-module.h"

namespace {

using namespace ViGraph::Dataflow;

//==========================================================================
// BitmapRender filter
class BitmapRender: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  BitmapRender *create_clone() const override
  {
    return new BitmapRender{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<int> width{1};
  Setting<int> height{1};
  Setting<int> pad_every{0};
  Setting<int> pad_extra{0};

  // Input
  Input<Bitmap::Group> input;

  // Output
  Output<DMXState> output;
};

//--------------------------------------------------------------------------
// Tick data
void BitmapRender::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(nsamples, tie(width, height), tie(input), tie(output),
                 [&](int width, int height,
                     const Bitmap::Group& input,
                     DMXState& output)
  {
    Bitmap::Rectangle bitmap(width, height);
    bitmap.fill(Colour::black);
    input.compose(bitmap);

    auto count = 0;
    for(const auto& pixel: bitmap.get_pixels())
    {
      output.channels.push_back(pixel.r * 255.0);
      output.channels.push_back(pixel.g * 255.0);
      output.channels.push_back(pixel.b * 255.0);

      if (pad_every && ++count >= pad_every)
      {
        output.channels.resize(output.channels.size()+pad_extra*3);
        count = 0;
      }
    }
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "bitmap-render",
  "Bitmap to DMX",
  "dmx",
  {
    { "width",  &BitmapRender::width  },
    { "height", &BitmapRender::height },
    { "pad-every", &BitmapRender::pad_every },
    { "pad-extra", &BitmapRender::pad_extra }
  },
  {
    { "input",  &BitmapRender::input  }
  },
  {
    { "output", &BitmapRender::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(BitmapRender, module)

