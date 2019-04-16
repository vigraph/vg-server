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
  RGB c;

  // Filter/Element virtuals
  void accept(FramePtr frame) override;

public:
  using FrameFilter::FrameFilter;

  // Getters/Setters
  string get() const { return c.str(); }
  void set(const string& colour) { c = RGB{colour}; }
  double get_r() const { return c.r; }
  void set_r(double r) { c.r = r; }
  double get_g() const { return c.g; }
  void set_g(double g) { c.g = g; }
  double get_b() const { return c.b; }
  void set_b(double b) { c.b = b; }
  double get_h() const { return HSL{c}.h; }
  void set_h(double h) { auto hsl = HSL{c}; hsl.h = h; c = RGB{hsl}; }
  double get_s() const { return HSL{c}.s; }
  void set_s(double s) { auto hsl = HSL{c}; hsl.s = s; c = RGB{hsl}; }
  double get_l() const { return HSL{c}.l; }
  void set_l(double l) { auto hsl = HSL{c}; hsl.l = l; c = RGB{hsl}; }
};

//--------------------------------------------------------------------------
// Process some data
void ColourFilter::accept(FramePtr frame)
{
  // Set all lit points to this colour
  for(auto& p: frame->points)
    if (p.is_lit()) p.c = c;

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
    { "l", { {"Lightnesss for HSL (0..1)","0.5"}, Value::Type::number,
             { &ColourFilter::get_l, &ColourFilter::set_l },
             true, true } },
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ColourFilter, module)
