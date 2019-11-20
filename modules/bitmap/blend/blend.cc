//==========================================================================
// ViGraph dataflow module: bitmap/blend/blend.cc
//
// Bitmap blend - creates a linear blended output between up to three single
// pixel input colours
//
// Copyright (c) 2018-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"

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

  // Internal
  bool get_colour(const Bitmap::Group& group, Colour::RGB& c);

public:
  using SimpleElement::SimpleElement;

  // Inputs
  Input<double> width{1};
  Input<double> height{1};
  Input<Bitmap::Group> top_left;
  Input<Bitmap::Group> top_right;
  Input<Bitmap::Group> bottom_left;
  Input<Bitmap::Group> bottom_right;

  // Output
  Output<Bitmap::Group> output;
};

//--------------------------------------------------------------------------
// Get colour of first pixel in first item in a group
// Returns whether successful and sets c if so
bool Blend::get_colour(const Bitmap::Group& group, Colour::RGB& c)
{
  if (group.items.empty()) return false;
  const auto& rect = group.items[0].rect;
  if (!rect.get_width() || !rect.get_height()) return false;
  c = rect(0,0);
  return true;
}

//--------------------------------------------------------------------------
// Generate a bitmap
void Blend::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(nsamples, {}, tie(width, height, top_left, top_right,
                                   bottom_left, bottom_right),
                 tie(output),
                 [&](double width, double height,
                     const Bitmap::Group& top_left,
                     const Bitmap::Group& top_right,
                     const Bitmap::Group& bottom_left,
                     const Bitmap::Group& bottom_right,
                     Bitmap::Group& output)
  {
    Bitmap::Rectangle rect(width, height);
    Colour::RGB top_left_c;
    if (!get_colour(top_left, top_left_c)) return;

    // Top right defaults top left if not set
    Colour::RGB top_right_c;
    if (!get_colour(top_right, top_right_c)) top_right_c = top_left_c;

    // Bottom left defaults to top left
    Colour::RGB bottom_left_c;
    if (!get_colour(bottom_left, bottom_left_c)) bottom_left_c = top_left_c;

    // Bottom right defaults to bottom left
    Colour::RGB bottom_right_c;
    if (!get_colour(bottom_right, bottom_right_c))
      bottom_right_c = bottom_left_c;

    // Blend both ways
    for(auto y=0; y<height; y++)
    {
      auto y_frac = (height>1) ? y/(height-1) : 0;
      auto start_c = top_left_c.blend_with(bottom_left_c, y_frac);
      auto end_c   = top_right_c.blend_with(bottom_right_c, y_frac);
      for(auto x=0; x<width; x++)
      {
        auto x_frac = (width>1) ? x/(width-1) : 0;
        auto c = start_c.blend_with(end_c, x_frac);
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
