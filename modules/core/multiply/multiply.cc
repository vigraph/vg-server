//==========================================================================
// ViGraph dataflow module: core/multiply/multiply.cc
//
// Multiplier module (=attenuator for audio)
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
  Input<double> input{0.0};
  Input<double> factor{1.0};
  Output<double> output;
};

//--------------------------------------------------------------------------
// Generate a fragment
void MultiplyFilter::tick(const TickData& td)
{
  sample_iterate(td.nsamples, {}, tie(input, factor), tie(output),
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
