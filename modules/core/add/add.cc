//==========================================================================
// ViGraph dataflow module: core/add/add.cc
//
// Multiplier module (=attenuator for audio)
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
  Input<double> input{0.0};
  Input<double> offset{0.0};
  Output<double> output;
};

//--------------------------------------------------------------------------
// Generate a fragment
void AddFilter::tick(const TickData& td)
{
  sample_iterate(td.nsamples, {}, tie(input, offset), tie(output),
                 [&](double input, double offset, double& o)
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
