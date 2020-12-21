//==========================================================================
// ViGraph dataflow module: control/knob/knob.cc
//
// Knob control
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"
#include <cmath>

namespace {

//==========================================================================
// KnobControl
class KnobControl: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  KnobControl *create_clone() const override
  {
    return new KnobControl{module};
  }

public:
  using SimpleElement::SimpleElement;

  Input<Number> value;

  // Output
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Tick data
void KnobControl::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(value), tie(output),
                 [&](Number value, Number& output)
  {
    output = value;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "knob",
  "Knob Control",
  "control",
  {},
  {
    { "value", &KnobControl::value }
  },
  {
    { "output", &KnobControl::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(KnobControl, module)
