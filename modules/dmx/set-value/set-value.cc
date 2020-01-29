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
  Input<Number> input;

  // Output
  Output<DMX::State> output;
};

//--------------------------------------------------------------------------
// Tick data
void SetValue::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input), tie(output),
                 [&](Number input,
                     DMX::State& output)
  {
    auto chan = DMX::channel_number(universe, channel);
    auto& channels = output.regions[chan];
    channels.push_back(input*255);
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
    { "channel", &SetValue::channel },
  },
  {
    { "input",  &SetValue::input  }
  },
  {
    { "output", &SetValue::output }
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SetValue, module)

