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

  // Configuration
  Input<double> input{0.0};
  Output<double> output;
  Output<double> enabled;
  Output<double> disabled;
};

//--------------------------------------------------------------------------
// Tick data
void Toggle::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(nsamples, {}, tie(input), tie(output, enabled, disabled),
                 [&](double i, double& o, double& e, double& d)
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
