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
  Setting<int> reverse_every{0};

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

    const auto& pixels = bitmap.get_pixels();
    for(auto count=0u; count<pixels.size(); count++)
    {
      auto i = count;

      // Allow reversal
      if (reverse_every)
      {
        const auto section = i/reverse_every;
        const auto section_start = section * reverse_every;
        if (!(section % 2)) // every other section
          i = section_start+reverse_every-(i-section_start);
      }

      const auto& pixel = pixels[i];
      output.channels.push_back(pixel.r * 255.0);
      output.channels.push_back(pixel.g * 255.0);
      output.channels.push_back(pixel.b * 255.0);

      if (pad_every && ++count >= (unsigned)pad_every)
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
    { "pad-extra", &BitmapRender::pad_extra },
    { "reverse-every", &BitmapRender::reverse_every }
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

