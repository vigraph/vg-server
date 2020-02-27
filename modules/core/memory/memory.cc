//==========================================================================
// ViGraph dataflow module: core/memory/memory.cc
//
// Provide a one-tick state memory for looped graphs
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"

namespace {

//==========================================================================
// Memory control
class Memory: public SimpleElement
{
 private:
  Number last_value{0};

  // Element virtuals
  void tick(const TickData& td) override;

  // Custom ready() so we can live in a loop
  bool ready() const override { return true; }

  // Read the input before it is reset, for next time
  void reset() override
  {
    // Check for reset first
    // Note this overrides even if an input comes in later
    const auto& rs = i_reset.get_buffer();
    for(auto r: rs)
    {
      if (r)
      {
        last_value = initial;
        return;
      }
    }

    const auto& in = input.get_buffer();
    if (in.size())
    {
      // Take average
      last_value = 0;
      for(auto v: in) last_value += v;
      last_value /= in.size();
    }

    Element::reset();
  }

  // We control our own input sample rate
  void update_sample_rate() override {}

  // Capture sample rate on setup
  void setup(const SetupContext& context) override
  {
    SimpleElement::setup(context);
    auto tick_interval = context.get_engine().get_tick_interval().seconds();
    if (tick_interval)
    {
      input.set_sample_rate(1.0/tick_interval);
      i_reset.set_sample_rate(1.0/tick_interval);
    }
    last_value = initial;
  }

  // Clone
  Memory *create_clone() const override
  {
    return new Memory{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<Number> initial{0.0};

  // Input
  Input<Number> input{0.0};
  Input<Trigger> i_reset{0};

  // Outputs
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Tick
void Memory::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();
  const auto nsamples = td.samples_in_tick(sample_rate);

  sample_iterate(td, nsamples, {}, {},
                 tie(output),
                 [&](Number& output)
  {
    output = last_value;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "memory",
  "Memory",
  "core",
  {
    { "initial", &Memory::initial }
  },
  {
    { "input", &Memory::input },
    { "reset", &Memory::i_reset }
  },
  {
    { "output", &Memory::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Memory, module)
