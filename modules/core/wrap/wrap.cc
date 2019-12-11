//==========================================================================
// ViGraph dataflow module: wrap/wrap.cc
//
// Control to wrap an input into a fixed range
//
// Copyright (c) 2018-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Wrap control
class Wrap: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Wrap *create_clone() const override
  {
    return new Wrap{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<double> min{0.0};
  Input<double> max{1.0};

  // Input
  Input<double> input{0.0};

  // Outputs
  Output<double> output;
};

//--------------------------------------------------------------------------
// Tick
void Wrap::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {},
                 tie(min, max, input),
                 tie(output),
                 [&](double min, double max, double input, double& output)
  {

    if (input >= max)
      output = min + fmod(input-min, max-min);
    else if (input < min)
      output = max - fmod(max-input, max-min);
    else
      output = input;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "wrap",
  "Wrap",
  "core",
  {},
  {
    { "min",   &Wrap::min },
    { "max",   &Wrap::max },
    { "input", &Wrap::input }
  },
  {
    { "output", &Wrap::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Wrap, module)
