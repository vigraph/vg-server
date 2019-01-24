//==========================================================================
// ViGraph dataflow module: filters/fade.cc
//
// Global brightness fade
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include <cmath>

namespace {

//==========================================================================
// Fade filter
class FadeFilter: public FrameFilter
{
  double fr{0.0};
  double fg{0.0};
  double fb{0.0};

  // Filter/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FramePtr frame) override;

public:
  // Construct
  FadeFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//  <fade all="0.5"/>
//  <fade r="0.9" g="1.0" b="1.1"/>
FadeFilter::FadeFilter(const Dataflow::Module *module,
                                 const XML::Element& config):
  Element(module, config), FrameFilter(module, config)
{
  if (config.has_attr("all"))
    fr = fg = fb = config.get_attr_real("all", 1.0);
  else
  {
    fr = config.get_attr_real("r", 1.0);
    fg = config.get_attr_real("g", 1.0);
    fb = config.get_attr_real("b", 1.0);
  }
}

//--------------------------------------------------------------------------
// Set a control property
void FadeFilter::set_property(const string& property, const SetParams& sp)
{
  if (property == "all")
  { update_prop(fr, sp); update_prop(fg, sp); update_prop(fb, sp); }
  else if (property == "r") update_prop(fr, sp);
  else if (property == "g") update_prop(fg, sp);
  else if (property == "b") update_prop(fb, sp);
}

//--------------------------------------------------------------------------
// Process some data
void FadeFilter::accept(FramePtr frame)
{
  // If fading to black, kill the frame altogether
  if (!fr && !fg && !fb)
  {
    frame->points.clear();
    send(frame);
    return;
  }

  // Modify all points in the frame
  for(auto& p: frame->points)
  {
    p.c.r *= fr;
    p.c.g *= fg;
    p.c.b *= fb;
  }

  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "fade",
  "Fade",
  "Fade all points to a multiple of their brightness",
  "vector",
  {
    { "all", { {"Factor to fade all channels","1.0"}, Value::Type::number,
                                                     "all", true } },
    { "r", { {"Factor to fade red","1.0"}, Value::Type::number, "r", true } },
    { "g", { {"Factor to fade green","1.0"}, Value::Type::number, "g", true } },
    { "b", { {"Factor to fade blue","1.0"}, Value::Type::number, "b", true } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(FadeFilter, module)
