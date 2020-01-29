//==========================================================================
// ViGraph dataflow module: dmx/set-value/set-value.cc
//
// Set a single DMX channel
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../dmx-module.h"

namespace {

using namespace ViGraph::Dataflow;

//==========================================================================
// SetValue filter
class SetValue: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  SetValue *create_clone() const override
  {
    return new SetValue{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Settings
  Setting<Integer> universe{0};
  Setting<Integer> channel{1};

  // Input
  Input<Number> value;    // 0..255
  Input<Number> input;    // 0..1

  // Output
  Output<DMX::State> output;
};

//--------------------------------------------------------------------------
// Tick data
void SetValue::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input, value), tie(output),
                 [&](Number _input, Number value,
                     DMX::State& output)
  {
    auto chan = DMX::channel_number(universe, channel);
    auto& channels = output.regions[chan];
    if (input.connected())
      channels.push_back(_input * DMX::max_value);
    else
      channels.push_back(value);
  });
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::SimpleModule module
{
  "set-value",
  "DMX channel output",
  "dmx",
  {
    { "universe", &SetValue::universe  },
    { "channel", &SetValue::channel }
  },
  {
    { "value", &SetValue::value },
    { "input",  &SetValue::input  }
  },
  {
    { "output", &SetValue::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SetValue, module)

