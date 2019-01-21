//==========================================================================
// ViGraph dataflow module: vector/filters/pattern/pattern.cc
//
// Filter to add complex colour patterns
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"

namespace {

//==========================================================================
// Pattern filter
class PatternFilter: public FrameFilter
{
  double phase{0};
  int repeats{1};
  vector<Colour::RGB> colours;

  // Filter/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FramePtr frame) override;

public:
  // Construct
  PatternFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//  <pattern phase="0" repeats="5" blend="linear">
//    <colour>red</colour>
//    <colour>black</colour>
//  </pattern>
PatternFilter::PatternFilter(const Dataflow::Module *module,
                             const XML::Element& config):
  Element(module, config), FrameFilter(module, config)
{
  phase = config.get_attr_real("phase");
  repeats = config.get_attr_int("repeats", 1);
  for(const auto& it: config.get_children("colour"))
  {
    const XML::Element& ce = *it;
    Colour::RGB c;
    if (!(*ce).empty())
      c = Colour::RGB(*ce);
    else if (ce.has_attr("h")) // assume HSL
      c = Colour::RGB(Colour::HSL(ce.get_attr_real("h"),
                                  ce.get_attr_real("s", 1.0),
                                  ce.get_attr_real("l", 0.5)));
    else
      c = Colour::RGB(ce.get_attr_real("r"),
                      ce.get_attr_real("g"),
                      ce.get_attr_real("b"));
    colours.push_back(c);
  }
}

//--------------------------------------------------------------------------
// Set a control property
void PatternFilter::set_property(const string& property, const SetParams& sp)
{
  if (property == "phase")
    update_prop(phase, sp);
  else if (property == "repeats")
    update_prop_int(repeats, sp);
}

//--------------------------------------------------------------------------
// Process some data
void PatternFilter::accept(FramePtr frame)
{
  auto nc = colours.size();
  if (nc)
  {
    // Count non-blanked points
    auto np = 0u;
    for(auto& p: frame->points)
      if (p.is_lit()) np++;

    if (np)
    {
      double i=0.0;
      for(auto& p: frame->points)
      {
        if (p.is_blanked()) continue;

        // Interpolate the colour map into the points, looping
        double frac = i/np + phase;
        double theta = frac*repeats;
        theta -= floor(theta);
        auto cindex = (unsigned int)floor(theta*nc);
        if (cindex < nc) p.c = colours[cindex];
        i++;
      }
    }
  }

  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "pattern",
  "Pattern",
  "Adds a pattern of colours, optionally blending",
  "vector",
  {
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(PatternFilter, module)
