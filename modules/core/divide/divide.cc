//==========================================================================
// ViGraph dataflow module: core/divide/divide.cc
//
// Divider module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../core-module.h"
#include <cfloat>

namespace {

//==========================================================================
// Divide filter
class DivideFilter: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  DivideFilter *create_clone() const override
  {
    return new DivideFilter{module};
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
void DivideFilter::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(nsamples, {}, tie(input, factor), tie(output),
                 [&](double input, double factor, double& o)
  {
    // If DBZ, fence to DBL_MAX/DBL_MIN to avoid sending NaN down the line
    o = factor ? (input/factor) : (input<0?DBL_MIN:DBL_MAX);
  });
}

Dataflow::SimpleModule module
{
  "divide",
  "Divide",
  "core",
  {},
  {
    { "input",  &DivideFilter::input },
    { "factor", &DivideFilter::factor }
  },
  {
    { "output", &DivideFilter::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(DivideFilter, module)
