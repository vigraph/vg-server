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
  Colour::RGB blank_colour = Colour::RGB{default_blank_colour};

  // Filter/Element virtuals
  void accept(FramePtr frame) override;

public:
  using FrameFilter::FrameFilter;

  // Getters/setters
  string get_colour() { return blank_colour.str(); }
  void set_colour(const string& colour) { blank_colour = Colour::RGB(colour); }
};

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
    { "colour", { "Colour to set blanked points to", Value::Type::text,
      { static_cast<string (Element::*)()>(&ShowBlankingFilter::get_colour),
        static_cast<void (Element::*)(const string&)>(
            &ShowBlankingFilter::set_colour) }, true } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ShowBlankingFilter, module)
