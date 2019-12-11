//==========================================================================
// ViGraph dataflow module: core/control-in/control-in.cc
//
// MIDI control in module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../midi-module.h"

namespace {

//==========================================================================
// ControlIn filter
class ControlIn: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  ControlIn *create_clone() const override
  {
    return new ControlIn{module};
  }

  Number last_output = 0.0;

public:
  using SimpleElement::SimpleElement;

  Input<Number> channel{-1};
  Input<Number> control{-1};
  Input<MIDI::Event> input;
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Generate a fragment
void ControlIn::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(channel, control, input), tie(output),
                 [&](Number c, Number co, const MIDI::Event& i, Number& o)
  {
    if (i.type == MIDI::Event::Type::control_change &&
        (c < 0 || i.channel == c) && (co < 0 || i.key == co))
      o = last_output = (i.value / 127.0);
    else
      o = last_output;
  });
}

Dataflow::SimpleModule module
{
  "control-in",
  "ControlIn",
  "midi",
  {},
  {
    { "channel",  &ControlIn::channel },
    { "control",  &ControlIn::control },
    { "input",    &ControlIn::input },
  },
  {
    { "output",   &ControlIn::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ControlIn, module)
