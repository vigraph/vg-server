//==========================================================================
// MIDI control out module
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../midi-module.h"

namespace {

//==========================================================================
// ControlOut filter
class ControlOut: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  ControlOut *create_clone() const override
  {
    return new ControlOut{module};
  }

  Number last_input = 0.0;

public:
  using SimpleElement::SimpleElement;

  Input<Number> channel{-1};
  Input<Number> control{-1};
  Input<Number> input{0};
  Output<MIDI::Event> output;
};

//--------------------------------------------------------------------------
// Generate a fragment
void ControlOut::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(channel, control, input), tie(output),
                 [&](Number c, Number co, Number i, MIDI::Event& o)
  {
    if (c >= 0 && co >= 0 && i != last_input)
      o = MIDI::Event(MIDI::Event::Type::control_change, c, co, i * 127.0);
    else
      o = MIDI::Event{};
    last_input = i;
  });
}

Dataflow::SimpleModule module
{
  "control-out",
  "Control Out",
  "midi",
  {},
  {
    { "channel",  &ControlOut::channel },
    { "control",  &ControlOut::control },
    { "input",    &ControlOut::input },
  },
  {
    { "output",   &ControlOut::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ControlOut, module)
