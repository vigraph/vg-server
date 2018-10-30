//==========================================================================
// ViGraph dataflow module: vector/filters/colour/colour.cc
//
// Simple filter which sets a fixed colour on its input
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"

namespace {

//==========================================================================
// Colour filter
class ColourFilter: public FrameFilter
{
  Colour::RGB c;

  // Filter/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FramePtr frame) override;

public:
  // Construct
  ColourFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//  <colour>#3055c0</colour>
//  <colour>rgb(100,100,255)</colour>
//  <colour>hsl(120,100,50)</colour>
//  <colour r='0.2' g='0.4' b='0.75'/>
//  <colour h='0.33' s='1' l='0.5'/>
ColourFilter::ColourFilter(const Dataflow::Module *module,
                           const XML::Element& config):
  Element(module, config), FrameFilter(module, config)
{
  if (!(*config).empty())
    c = Colour::RGB(*config);
  else if (config.has_attr("h")) // assume HSL
    c = Colour::RGB(Colour::HSL(config.get_attr_real("h"),
                                config.get_attr_real("s", 1.0),
                                config.get_attr_real("l", 0.5)));
  else
    c = Colour::RGB(config.get_attr_real("r"),
                    config.get_attr_real("g"),
                    config.get_attr_real("b"));
}

//--------------------------------------------------------------------------
// Set a control property
void ColourFilter::set_property(const string& property, const SetParams& sp)
{
       if (property == "r") update_prop(c.r, sp);
  else if (property == "g") update_prop(c.g, sp);
  else if (property == "b") update_prop(c.b, sp);
  else if (property == "h")
  {
    // Convert to HSL, modify and convert back
    Colour::HSL hsl(c);
    update_prop(hsl.h, sp);
    c = Colour::RGB(hsl);
  }
  else if (property == "s")
  { Colour::HSL hsl(c); update_prop(hsl.s, sp); c = Colour::RGB(hsl); }
  else if (property == "l")
  { Colour::HSL hsl(c); update_prop(hsl.l, sp); c = Colour::RGB(hsl); }
}

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
    { "", { "Colour text #rrbbgg or rgb(r,g,b) or hsl(h,s,l)", Value::Type::text,
          "" } },
    { "r", { "Red component (0..1)", Value::Type::number, "@r", true } },
    { "g", { "Green component (0..1)", Value::Type::number, "@g", true } },
    { "b", { "Blue component (0..1)", Value::Type::number, "@b", true } },
    { "h", { "Hue for HSL (0..1)", Value::Type::number, "@h", true } },
    { "s", { {"Saturation for HSL (0..1)","1.0"}, Value::Type::number, "@s", true}},
    { "l", { {"Lightnesss for HSL (0..1)","0.5"}, Value::Type::number, "@l", true}}
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ColourFilter, module)
