//==========================================================================
// ViGraph dataflow module: dmx/get-value/get-value.cc
//
// Get a single DMX channel
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../dmx-module.h"

namespace {

using namespace ViGraph::Dataflow;

//==========================================================================
// GetValue filter
class GetValue: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  GetValue *create_clone() const override
  {
    return new GetValue{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<Integer> universe{0};
  Setting<Integer> channel{1};

  // Input
  Input<DMX::State> input;

  // Output
  Output<Number> output; // 0..1
};

//--------------------------------------------------------------------------
// Tick data
void GetValue::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input), tie(output),
                 [&](const DMX::State& input,
                     Number& output)
  {
    auto chan = DMX::channel_number(universe, channel);
    output = static_cast<Number>(input.get(chan)) / DMX::max_value;
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "get-value",
  "DMX value input",
  "dmx",
  {
    { "universe", &GetValue::universe  },
    { "channel", &GetValue::channel },
  },
  {
    { "input",  &GetValue::input  }
  },
  {
    { "output", &GetValue::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(GetValue, module)

