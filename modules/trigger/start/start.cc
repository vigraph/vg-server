//==========================================================================
// ViGraph dataflow module: trigger/start/start.cc
//
// Control to just trigger once at startup
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"

namespace {

//==========================================================================
// Start control
class Start: public SimpleElement
{
 private:
  bool triggered{false};

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Start *create_clone() const override
  {
    return new Start{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Outputs
  Output<Trigger> output;
};

//--------------------------------------------------------------------------
// Tick
void Start::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, {},
                 tie(output),
                 [&](Trigger& output)
  {
    if (!triggered)
    {
      output = 1;
      triggered = true;
    }
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "start",
  "Start trigger",
  "trigger",
  {},
  {},
  {
    { "output", &Start::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Start, module)
