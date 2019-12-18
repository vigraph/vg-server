//==========================================================================
// ViGraph dataflow module: audio/level/level.cc
//
// Audio level module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../audio-module.h"

namespace {

//==========================================================================
// Level filter
class LevelFilter: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  LevelFilter *create_clone() const override
  {
    return new LevelFilter{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<Number> input{0.0};
  Input<Number> gain{1.0};
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Generate a fragment
void LevelFilter::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(input, gain), tie(output),
                 [&](Number input, Number gain, Number& o)
  {
    o = input * gain;
  });
}

Dataflow::SimpleModule module
{
  "level",
  "Level",
  "audio",
  {},
  {
    { "input", &LevelFilter::input },
    { "gain",  &LevelFilter::gain  }
  },
  {
    { "output", &LevelFilter::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(LevelFilter, module)
