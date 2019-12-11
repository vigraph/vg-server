//==========================================================================
// ViGraph dataflow module: core/cut/cut.cc
//
// Control to provide a break point for trigger loops
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"

namespace {

//==========================================================================
// Cut control
class Cut: public SimpleElement
{
 private:
  bool was_triggered{false};

  // Element virtuals
  void tick(const TickData& td) override;

  // Custom ready() so we can live in a loop
  bool ready() const override { return true; }

  // Read the input before it is reset, for next time
  void reset() override
  {
    const auto& in = input.get_buffer();
    for(auto i: in)
      if (i) was_triggered = true;
    Element::reset();
  }

  // Clone
  Cut *create_clone() const override
  {
    return new Cut{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Triggers
  Input<Trigger> input{0.0};

  // Outputs
  Output<Trigger> output;
};

//--------------------------------------------------------------------------
// Tick
void Cut::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();
  const auto nsamples = td.samples_in_tick(sample_rate);
  auto trigger{was_triggered};
  if (nsamples) was_triggered = false;

  sample_iterate(td, nsamples, {}, {},
                 tie(output),
                 [&](Trigger& output)
  {
    // Output from previous state only on first sample
    if (trigger)
    {
      output = 1;
      trigger = false;
    }
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "cut",
  "Loop Cut",
  "core",
  {},
  {
    { "input", &Cut::input }
  },
  {
    { "output", &Cut::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Cut, module)
