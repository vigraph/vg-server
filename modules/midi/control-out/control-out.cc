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
  Output<MIDIEvents> output;
};

//--------------------------------------------------------------------------
// Generate a fragment
void ControlOut::tick(const TickData& td)
{
  const auto sample_rate = input.get_sample_rate();
  const auto nsamples = td.samples_in_tick(sample_rate);
  const auto sample_duration = Time::Duration{td.sample_duration(sample_rate)};
  auto t = Time::Duration{td.start};
  sample_iterate(td, nsamples, {}, tie(channel, control, input), tie(output),
                 [&](Number c, Number co, Number i, MIDIEvents& o)
  {
    o = MIDIEvents{};
    if (c >= 0 && co >= 0 && i != last_input)
      o.emplace_back(t, MIDI::Event::Type::control_change, c, co, i * 127.0);
    last_input = i;
    t += sample_duration;
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
