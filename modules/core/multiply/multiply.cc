//==========================================================================
// ViGraph dataflow module: core/multiply/multiply.cc
//
// Multiplier module (=attenuator and ring mod for audio)
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../core-module.h"

namespace {

//==========================================================================
// Multiply filter
class MultiplyFilter: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  MultiplyFilter *create_clone() const override
  {
    return new MultiplyFilter{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<Number> input{0.0};
  Input<Number> factor{1.0};
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Generate a fragment
void MultiplyFilter::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input, factor), tie(output),
                 [&](double input, double factor, double& o)
  {
    o = input * factor;
  });
}

Dataflow::SimpleModule module
{
  "multiply",
  "Multiply",
  "core",
  {},
  {
    { "input",        &MultiplyFilter::input },
    { "factor",       &MultiplyFilter::factor }
  },
  {
    { "output", &MultiplyFilter::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MultiplyFilter, module)
