//==========================================================================
// ViGraph dataflow module: core/add/add.cc
//
// Adder module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../core-module.h"

namespace {

//==========================================================================
// Add filter
class AddFilter: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  AddFilter *create_clone() const override
  {
    return new AddFilter{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<Number> input{0.0};
  Input<Number> offset{0.0};
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Tick data
void AddFilter::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input, offset), tie(output),
                 [&](Number input, Number offset, Number& o)
  {
    o = input + offset;
  });
}

Dataflow::SimpleModule module
{
  "add",
  "Add",
  "core",
  {},
  {
    { "input",        &AddFilter::input },
    { "offset",       &AddFilter::offset }
  },
  {
    { "output", &AddFilter::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AddFilter, module)
