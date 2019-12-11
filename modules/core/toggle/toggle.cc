//==========================================================================
// ViGraph dataflow module: core/toggle/toggle.cc
//
// Toggle module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../core-module.h"

namespace {

//==========================================================================
// Toggle filter
class Toggle: public SimpleElement
{
private:
  bool on = false;

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Toggle *create_clone() const override
  {
    return new Toggle{module};
  }

public:
  using SimpleElement::SimpleElement;

  Input<Trigger> input{0.0};
  Output<Number> output;
  Output<Trigger> enabled;
  Output<Trigger> disabled;
};

//--------------------------------------------------------------------------
// Tick data
void Toggle::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input), tie(output, enabled, disabled),
                 [&](Trigger i, Number& o, Trigger& e, Trigger& d)
  {
    e = d = 0;
    if (i)
    {
      if (on)
        d = 1;
      else
        e = 1;
      on = !on;
    }
    o = on ? 1 : 0;
  });
}

Dataflow::SimpleModule module
{
  "toggle",
  "Toggle",
  "core",
  {},
  {
    { "input",    &Toggle::input },
  },
  {
    { "output",   &Toggle::output },
    { "enabled",  &Toggle::enabled },
    { "disabled", &Toggle::disabled },
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Toggle, module)
