//==========================================================================
// ViGraph dataflow module: core/subtract/subtract.cc
//
// Subtractor module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../core-module.h"

namespace {

//==========================================================================
// Subtract filter
class SubtractFilter: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  SubtractFilter *create_clone() const override
  {
    return new SubtractFilter{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<Number> input{0.0};
  Input<Number> offset{0.0};
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Generate a fragment
void SubtractFilter::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input, offset), tie(output),
                 [&](Number input, Number offset, Number& o)
  {
    o = input - offset;
  });
}

Dataflow::SimpleModule module
{
  "subtract",
  "Subtract",
  "core",
  {},
  {
    { "input",        &SubtractFilter::input },
    { "offset",       &SubtractFilter::offset }
  },
  {
    { "output", &SubtractFilter::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SubtractFilter, module)
