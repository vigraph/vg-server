//==========================================================================
// ViGraph dataflow module: control/slider/slider.cc
//
// Slider control
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"
#include <cmath>

namespace {

//==========================================================================
// SliderControl
class SliderControl: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  SliderControl *create_clone() const override
  {
    return new SliderControl{module};
  }

public:
  using SimpleElement::SimpleElement;

  Input<Number> value;

  // Output
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Tick data
void SliderControl::tick(const TickData& td)
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
  "slider",
  "Slider Control",
  "control",
  {},
  {
    { "value", &SliderControl::value }
  },
  {
    { "output", &SliderControl::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SliderControl, module)
