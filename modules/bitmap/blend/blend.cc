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
void Blend::setup(const SetupContext& context)
{
  DynamicElement::setup(context);

  // Do nothing if same type
  if (type == last_type) return;

  // Get rid of old inputs
  switch (last_type)
  {
    case BlendType::none: break;

    case BlendType::horizontal:
      deregister_input("left", &left);
      deregister_input("right", &right);
      break;

    case BlendType::vertical:
      deregister_input("top", &top);
      deregister_input("bottom", &bottom);
      break;

    case BlendType::rectangular:
      deregister_input("top-left", &top_left);
      deregister_input("top-right", &top_right);
      deregister_input("bottom-left", &bottom_left);
      deregister_input("bottom-right", &bottom_right);
      break;

    case BlendType::radial:
      deregister_input("centre", &centre);
      deregister_input("edge", &edge);
      deregister_input("radius", &radius);
      break;
  }

  // Add new inputs
  switch (type)
  {
    case BlendType::none: break;

    case BlendType::horizontal:
      register_input("left", &left);
      register_input("right", &right);
      break;

    case BlendType::vertical:
      register_input("top", &top);
      register_input("bottom", &bottom);
      break;

    case BlendType::rectangular:
      register_input("top-left", &top_left);
      register_input("top-right", &top_right);
      register_input("bottom-left", &bottom_left);
      register_input("bottom-right", &bottom_right);
      break;

    case BlendType::radial:
      register_input("centre", &centre);
      register_input("edge",   &edge);
      register_input("radius", &radius);
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
