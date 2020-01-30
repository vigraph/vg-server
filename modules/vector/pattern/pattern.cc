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
  vector<shared_ptr<Input<Number>>> proportion_list;

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
  while (proportion_list.size() > ncolours)
  {
    const auto i = proportion_list.size();
    deregister_input("proportion" + Text::itos(i),
                     proportion_list.back().get());
    proportion_list.pop_back();
  }
  while (proportion_list.size() < ncolours)
  {
    proportion_list.emplace_back(new Input<Number>{1});
    const auto i = proportion_list.size();
    register_input("proportion" + Text::itos(i), proportion_list.back().get());
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
  auto proportions = vector<pair<const Input<Number> *,
                                 const vector<Number> *>>{};
  for (const auto& prop: proportion_list)
    proportions.emplace_back(prop.get(), &prop->get_buffer());
  vector<Number> props(nc);
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
          auto total_props = Number{};
          auto pi = 0u;
          for (const auto& prop: proportions)
          {
            const auto val = max(numeric_limits<Number>::min(),
                                 safe_input_buffer_get(*prop.first,
                                                       *prop.second, c));
            props[pi++] = val;
            total_props += val;
          }
          auto cindex = 0u;
          auto pt = Number{};
          for (const auto& prop: props)
          {
            if (pt + prop > theta * total_props)
              break;
            pt += prop;
            ++cindex;
          }
          if (cindex >= nc)
          {
            p.c = Colour::black;  // Double safety
            continue;
          }

          const auto colour = safe_input_buffer_get(*colour_list[cindex],
                                                    *colours[cindex], c);
          if (blend)
          {
            auto next_cindex = (cindex+1)%nc;
            auto factor = (theta * total_props - pt) / props[cindex];
            const auto next_colour = safe_input_buffer_get(
                                                *colour_list[next_cindex],
                                                *colours[next_cindex], c);
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
