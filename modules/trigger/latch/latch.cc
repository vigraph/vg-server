//==========================================================================
// ViGraph dataflow module: trigger/latch/latch.cc
//
// Latch module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../trigger-module.h"

namespace {

//==========================================================================
// Latch filter
class Latch: public SimpleElement
{
private:
  bool on = false;

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Latch *create_clone() const override
  {
    return new Latch{module};
  }

public:
  using SimpleElement::SimpleElement;

  Input<Trigger> set{0.0};
  Input<Trigger> clear{0.0};
  Input<Trigger> toggle{0.0};

  Output<Number> output;
  Output<Trigger> enabled;
  Output<Trigger> disabled;
};

//--------------------------------------------------------------------------
// Tick data
void Latch::tick(const TickData& td)
{
  const auto sample_rate = max(output.get_sample_rate(),
                               max(enabled.get_sample_rate(),
                                   disabled.get_sample_rate()));
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {},
                 tie(set, clear, toggle),
                 tie(output, enabled, disabled),
                 [&](Trigger set, Trigger clear, Trigger toggle,
                     Number& o, Trigger& e, Trigger& d)
  {
    e = d = 0;
    if (set)
    {
      e = 1;
      on = true;
    }
    else if (clear)
    {
      d = 1;
      on = false;
    }
    else if (toggle)
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
  "latch",
  "Latch",
  "trigger",
  {},
  {
    { "set",    &Latch::set    },
    { "clear",  &Latch::clear  },
    { "toggle", &Latch::toggle },
  },
  {
    { "output",   &Latch::output },
    { "enabled",  &Latch::enabled },
    { "disabled", &Latch::disabled },
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Latch, module)
