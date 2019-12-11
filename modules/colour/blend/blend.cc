//==========================================================================
// ViGraph dataflow module: colour/blend/blend.cc
//
// Blend colour filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../colour-module.h"
#include <cmath>

namespace {

//==========================================================================
// Blend
class Blend: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Blend *create_clone() const override
  {
    return new Blend{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<double> mix{0.0};

  // Inputs
  Input<Colour::RGB> from;
  Input<Colour::RGB> to;

  // Output
  Output<Colour::RGB> output;
};

//--------------------------------------------------------------------------
// Tick data
void Blend::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(mix, from, to), tie(output),
                 [&](double mix, const Colour::RGB& from,
                     const Colour::RGB& to,
                     Colour::RGB& output)
  {
    output = from.blend_with(to, mix);
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "blend",
  "Blend Colour",
  "colour",
  {},
  {
    { "mix",  &Blend::mix  },
    { "from", &Blend::from },
    { "to",   &Blend::to   }
  },
  {
    { "output", &Blend::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Blend, module)
