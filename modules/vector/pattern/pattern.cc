//==========================================================================
// Vector pattern module
//
// Copyright (c) 2017-2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include "../../colour/colour-module.h"
#include <cmath>

namespace {

//==========================================================================
// Pattern
class Pattern: public DynamicElement
{
private:
  vector<shared_ptr<Input<Colour::RGB>>> colour_list;

  // Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;

  // Clone
  Pattern *create_clone() const override
  {
    return new Pattern{module};
  }

public:
  using DynamicElement::DynamicElement;

  // Configuration
  Setting<Integer> colours{1};
  Setting<bool> blend{false};

  // Input
  Input<Number> phase{0};
  Input<Number> repeats{1.0};
  Input<Frame> input;

  // Output
  Output<Frame> output;
};

//--------------------------------------------------------------------------
// Setup
void Pattern::setup(const SetupContext& context)
{
  DynamicElement::setup(context);

  const auto ncolours = static_cast<unsigned>(max(colours.get(), 0l));
  while (colour_list.size() > ncolours)
  {
    const auto i = colour_list.size();
    deregister_input("colour" + Text::itos(i), colour_list.back().get());
    colour_list.pop_back();
  }
  while (colour_list.size() < ncolours)
  {
    colour_list.emplace_back(new Input<Colour::RGB>{1});
    const auto i = colour_list.size();
    register_input("colour" + Text::itos(i), colour_list.back().get());
  }
}

//--------------------------------------------------------------------------
// Tick data
void Pattern::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  const auto nc = colour_list.size();
  auto colours = vector<const vector<Colour::RGB> *>{};
  for (const auto& col: colour_list)
    colours.emplace_back(&col->get_buffer());
  auto c = 0u;
  sample_iterate(td, nsamples, {}, tie(phase, repeats, input), tie(output),
                 [&](Number phase, Number repeats, const Frame& input,
                     Frame& output)
  {
    output = input;
    if (nc)
    {
      auto np = 0u;
      for (auto& p: output.points)
        if (p.is_lit())
          ++np;

      if (np)
      {
        auto i = double{};
        for (auto& p: output.points)
        {
          if (p.is_blanked()) continue;

          // Interpolate the colour map into the points, looping
          auto frac = i / np;
          auto theta = frac * repeats + phase;
          theta -= floor(theta);

          // Theta can become 1.0 due to rounding in the above
          if (theta >= 1.0) theta = 1.0 - 1e-10;
          auto cindex = static_cast<unsigned int>(floor(theta*nc));
          if (cindex >= nc)
          {
            p.c = Colour::black;  // Double safety
            continue;
          }

          const auto colour = colours[cindex]->size() < c
                              ? (*colours[cindex])[c]
                              : colour_list[cindex]->get();
          if (blend)
          {
            auto next_cindex = (cindex+1)%nc;
            auto factor = theta*nc - cindex;
            const auto next_colour = colours[next_cindex]->size() < c
                                     ? (*colours[next_cindex])[c]
                                     : colour_list[next_cindex]->get();
            p.c = colour.blend_with(next_colour, factor);
          }
          else
          {
            p.c = colour;
          }
          ++i;
        }
      }
    }
    ++c;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::DynamicModule module
{
  "pattern",
  "Pattern",
  "vector",
  {
    { "colours", &Pattern::colours },
    { "blend", &Pattern::blend },
  },
  {
    { "phase", &Pattern::phase },
    { "repeats", &Pattern::repeats },
    { "input", &Pattern::input },
  },
  {
    { "output", &Pattern::output },
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Pattern, module)
