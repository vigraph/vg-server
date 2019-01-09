//==========================================================================
// ViGraph dataflow module: laser/filters/show-blanking/show-blanking.cc
//
// Filter to show blanked points
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../vector/vector-module.h"

namespace {

const auto default_blank_colour = "red";

//==========================================================================
// ShowBlanking filter
class ShowBlankingFilter: public FrameFilter
{
  Colour::RGB blank_colour;

  // Filter/Element virtuals
  void accept(FramePtr frame) override;

public:
  // Construct
  ShowBlankingFilter(const Dataflow::Module *module,
                     const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//  <show-blanking colour="red"/>
ShowBlankingFilter::ShowBlankingFilter(const Dataflow::Module *module,
                                       const XML::Element& config):
  Element(module, config), FrameFilter(module, config)
{
  blank_colour = Colour::RGB(config.get_attr("colour", default_blank_colour));
}

//--------------------------------------------------------------------------
// Process some data
void ShowBlankingFilter::accept(FramePtr frame)
{
  for(auto& p: frame->points)
    if (p.is_blanked()) p.c = blank_colour;

  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "show-blanking",
  "Show Blanking",
  "Show blanked points",
  "laser",
  {
    { "colour", { {"Colour to set blanked points to", "red"},
          Value::Type::text } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ShowBlankingFilter, module)
