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
#include "blend-type.h"

namespace {

//==========================================================================
// Blend
class Blend: public DynamicElement
{
private:
  BlendType last_type{BlendType::none};

  // Internal
  void tick_horizontal(const TickData& td);
  void tick_vertical(const TickData& td);
  void tick_rectangular(const TickData& td);
  void tick_radial(const TickData& td);

  // Element virtuals
  void tick(const TickData& td) override;
  void setup(const SetupContext& context) override;

  // Clone
  Blend *create_clone() const override
  {
    return new Blend{module};
  }

public:
  using DynamicElement::DynamicElement;

  // Settings
  Setting<BlendType> type{BlendType::none};

  // Inputs
  Input<Number> width{1};
  Input<Number> height{1};

  // Horizontal
  Input<Colour::RGB> left{Colour::black};
  Input<Colour::RGB> right{Colour::black};

  // Vertical
  Input<Colour::RGB> top{Colour::black};
  Input<Colour::RGB> bottom{Colour::black};

  // Rectangular
  Input<Colour::RGB> top_left{Colour::black};
  Input<Colour::RGB> top_right{Colour::black};
  Input<Colour::RGB> bottom_left{Colour::black};
  Input<Colour::RGB> bottom_right{Colour::black};

  // Radial
  Input<Colour::RGB> centre{Colour::black};
  Input<Colour::RGB> edge{Colour::black};
  Input<Number> radius{0.5};

  // Output
  Output<Bitmap::Group> output;
};

//--------------------------------------------------------------------------
// Setup inputs
void Blend::setup(const SetupContext&)
{
  // Do nothing if same type
  if (type == last_type) return;

  // Get rid of old inputs
  switch (last_type)
  {
    case BlendType::none: break;

    case BlendType::horizontal:
      module.erase_input("left");
      module.erase_input("right");
      break;

    case BlendType::vertical:
      module.erase_input("top");
      module.erase_input("bottom");
      break;

    case BlendType::rectangular:
      module.erase_input("top-left");
      module.erase_input("top-right");
      module.erase_input("bottom-left");
      module.erase_input("bottom-right");
      break;

    case BlendType::radial:
      module.erase_input("centre");
      module.erase_input("edge");
      module.erase_input("radius");
      break;
  }

  // Add new inputs
  switch (type)
  {
    case BlendType::none: break;

    case BlendType::horizontal:
      module.add_input("left", &Blend::left);
      module.add_input("right", &Blend::right);
      break;

    case BlendType::vertical:
      module.add_input("top", &Blend::top);
      module.add_input("bottom", &Blend::bottom);
      break;

    case BlendType::rectangular:
      module.add_input("top-left", &Blend::top_left);
      module.add_input("top-right", &Blend::top_right);
      module.add_input("bottom-left", &Blend::bottom_left);
      module.add_input("bottom-right", &Blend::bottom_right);
      break;

    case BlendType::radial:
      module.add_input("centre", &Blend::centre);
      module.add_input("edge",   &Blend::edge);
      module.add_input("radius", &Blend::radius);
      break;
  }

  last_type = type;
}

//--------------------------------------------------------------------------
// Horizontal blend
void Blend::tick_horizontal(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(width, height, left, right),
                 tie(output),
                 [&](Number width, Number height,
                     const Colour::RGB& left,
                     const Colour::RGB& right,
                     Bitmap::Group& output)
  {
    Bitmap::Rectangle rect(width, height);

    // Blend horizontally, repeated for all columns
    for(auto x=0; x<width; x++)
    {
      auto x_frac = (width>1) ? x/(width-1) : 0;
      auto c = left.blend_with(right, x_frac);
      for(auto y=0; y<height; y++)
        rect.set(x, y, c);
    }

    output.add(rect);
  });
}

//--------------------------------------------------------------------------
// Vertical blend
void Blend::tick_vertical(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(width, height, top, bottom),
                 tie(output),
                 [&](Number width, Number height,
                     const Colour::RGB& top,
                     const Colour::RGB& bottom,
                     Bitmap::Group& output)
  {
    Bitmap::Rectangle rect(width, height);

    // Blend vertically, repeated for all rows
    for(auto y=0; y<height; y++)
    {
      auto y_frac = (height>1) ? y/(height-1) : 0;
      auto c = top.blend_with(bottom, y_frac);
      for(auto x=0; x<width; x++)
        rect.set(x, y, c);
    }

    output.add(rect);
  });
}

//--------------------------------------------------------------------------
// Rectangular blend
void Blend::tick_rectangular(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(width, height, top_left, top_right,
                                   bottom_left, bottom_right),
                 tie(output),
                 [&](Number width, Number height,
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
// Radial blend
void Blend::tick_radial(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(width, height, centre, edge, radius),
                 tie(output),
                 [&](Number width, Number height,
                     const Colour::RGB& centre,
                     const Colour::RGB& edge,
                     Number radius,
                     Bitmap::Group& output)
  {
    Bitmap::Rectangle rect(width, height);

    for(auto y=0; y<height; y++)
    {
      for(auto x=0; x<width; x++)
      {
        Point p(x, y);
        Point o(width/2, height/2);
        auto d = p.distance_to(o);
        auto f = min(1.0, (radius>0)?(d/radius/width):1.0);
        auto c = centre.blend_with(edge, f);
        rect.set(x, y, c);
      }
    }

    output.add(rect);
  });
}

//--------------------------------------------------------------------------
// Generate a bitmap
void Blend::tick(const TickData& td)
{
  switch (type)
  {
    case BlendType::none: break;

    case BlendType::horizontal:
      tick_horizontal(td);
      break;

    case BlendType::vertical:
      tick_vertical(td);
      break;

    case BlendType::rectangular:
      tick_rectangular(td);
      break;

    case BlendType::radial:
      tick_radial(td);
      break;
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::DynamicModule module
{
  "blend",
  "Bitmap blend",
  "bitmap",
  {
    { "type", &Blend::type }
  },
  {
    { "width",        &Blend::width        },
    { "height",       &Blend::height       }
  },
  {
    { "output",    &Blend::output    }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Blend, module)
