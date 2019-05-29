//==========================================================================
// ViGraph dataflow module: vector/filters/colour/colour.cc
//
// Simple filter which sets a fixed colour on its input
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"

namespace {

using Colour::RGB;
using Colour::HSL;

//==========================================================================
// Colour filter
class ColourFilter: public FrameFilter
{
  RGB rgb;
  HSL hsl;  // Kept separately for HSL updates, otherwise order dependent

  // Filter/Element virtuals
  void accept(FramePtr frame) override;

public:
  using FrameFilter::FrameFilter;

  // Getters/Setters
  string get() const { return rgb.str(); }
  void set(const string& colour) { rgb = RGB{colour}; hsl=HSL{rgb}; }
  double get_r() const { return rgb.r; }
  void set_r(double r) { rgb.r = r; hsl=HSL{rgb}; }
  double get_g() const { return rgb.g; }
  void set_g(double g) { rgb.g = g; hsl=HSL{rgb}; }
  double get_b() const { return rgb.b; }
  void set_b(double b) { rgb.b = b; hsl=HSL{rgb}; }
  double get_h() const { return hsl.h; }
  void set_h(double h) { hsl.h = h; rgb = RGB{hsl}; }
  double get_s() const { return hsl.s; }
  void set_s(double s) { hsl.s = s; rgb = RGB{hsl}; }
  double get_l() const { return hsl.l; }
  void set_l(double l) { hsl.l = l; rgb = RGB{hsl}; }
};

//--------------------------------------------------------------------------
// Process some data
void ColourFilter::accept(FramePtr frame)
{
  // Set all lit points to this colour
  for(auto& p: frame->points)
    if (p.is_lit()) p.c = rgb;

  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "colour",
  "Colour",
  "Sets the colour of the frame to RGB or HSL",
  "vector",
  {
    { "hex", { "Colour hex [#]rrggbb or [#]rgb",
            Value::Type::text,
            { &ColourFilter::get, &ColourFilter::set },
            true } },
    { "name", { "Colour name",
            Value::Type::text,
            { &ColourFilter::get, &ColourFilter::set },
            false, true } },
    { "r", { "Red component (0..1)", Value::Type::number,
             { &ColourFilter::get_r, &ColourFilter::set_r },
             true, true } },
    { "g", { "Green component (0..1)", Value::Type::number,
             { &ColourFilter::get_g, &ColourFilter::set_g },
             true, true } },
    { "b", { "Blue component (0..1)", Value::Type::number,
             { &ColourFilter::get_b, &ColourFilter::set_b },
             true, true } },
    { "h", { "Hue for HSL (0..1)", Value::Type::number,
             { &ColourFilter::get_h, &ColourFilter::set_h },
             true, true } },
    { "s", { "Saturation for HSL (0..1)", Value::Type::number,
             { &ColourFilter::get_s, &ColourFilter::set_s },
             true, true } },
    { "l", { "Lightness for HSL (0..1)", Value::Type::number,
             { &ColourFilter::get_l, &ColourFilter::set_l },
             true, true } },
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ColourFilter, module)
