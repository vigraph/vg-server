//==========================================================================
// ViGraph dataflow module: laser/show-blanking/show-blanking.cc
//
// Filter to show blanked points

// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector/vector-module.h"

namespace {

const auto default_blank_colour = "red";

//==========================================================================
// Show Blanking
class ShowBlanking: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  ShowBlanking *create_clone() const override
  {
    return new ShowBlanking{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Setting<string> blank_colour{default_blank_colour};

  // Input
  Input<Frame> input;

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Tick data
void ShowBlanking::tick(const TickData& td)
{
  Colour::RGB bc(blank_colour);
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input), tie(output),
                 [&](const Frame& input, Frame& output)
  {
    output = input;

    for(auto& p: output.points)
      if (p.is_blanked()) p.c = bc;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "show-blanking",
  "Show blanking",
  "laser",
  {
    { "colour", &ShowBlanking::blank_colour }
  },
  {
    { "input", &ShowBlanking::input }
  },
  {
    { "output", &ShowBlanking::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ShowBlanking, module)
