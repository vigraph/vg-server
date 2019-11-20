//==========================================================================
// ViGraph dataflow module: bitmap/blend/blend.cc
//
// Bitmap blend - creates a linear blended output between up to three single
// pixel input colours
//
// Copyright (c) 2018-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"
#include "../../colour/colour-module.h"

namespace {

//==========================================================================
// Blend
class Blend: public SimpleElement
{
private:
  // /Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Blend *create_clone() const override
  {
    return new Blend{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Inputs
  Input<double> width{1};
  Input<double> height{1};
  Input<Colour::RGB> top_left;
  Input<Colour::RGB> top_right;
  Input<Colour::RGB> bottom_left;
  Input<Colour::RGB> bottom_right;

  // Output
  Output<Bitmap::Group> output;
};

//--------------------------------------------------------------------------
// Generate a bitmap
void Blend::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(nsamples, {}, tie(width, height, top_left, top_right,
                                   bottom_left, bottom_right),
                 tie(output),
                 [&](double width, double height,
                     const Colour::RGB& top_left,
                     const Colour::RGB& top_right,
                     const Colour::RGB& bottom_left,
                     const Colour::RGB& bottom_right,
                     Bitmap::Group& output)
  {
    Bitmap::Rectangle rect(width, height);

    // Blend both ways
    for(auto y=0; y<height; y++)
    {
      auto y_frac = (height>1) ? y/(height-1) : 0;
      auto start = top_left.blend_with(bottom_left, y_frac);
      auto end   = top_right.blend_with(bottom_right, y_frac);
      for(auto x=0; x<width; x++)
      {
        auto x_frac = (width>1) ? x/(width-1) : 0;
        auto c = start.blend_with(end, x_frac);
        rect.set(x, y, c);
      }
    }

    output.add(rect);
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "blend",
  "Bitmap blend",
  "bitmap",
  {},
  {
    { "width",        &Blend::width        },
    { "height",       &Blend::height       },
    { "top-left",     &Blend::top_left     },
    { "top-right",    &Blend::top_right    },
    { "bottom-left",  &Blend::bottom_left  },
    { "bottom-right", &Blend::bottom_right }
  },
  {
    { "output",    &Blend::output    }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Blend, module)
