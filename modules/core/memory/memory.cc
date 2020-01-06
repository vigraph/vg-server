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
    const auto& in = input.get_buffer();
    if (in.size()) last_value = in.back();
    Element::reset();
  }

  // Clone
  Memory *create_clone() const override
  {
    return new Memory{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Input
  Input<Number> input{0.0};

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
  {},
  {
    { "input", &Memory::input }
  },
  {
    { "output", &Memory::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Memory, module)
