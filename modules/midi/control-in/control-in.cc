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
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;

  // Clone
  ControlIn *create_clone() const override
  {
    return new ControlIn{module};
  }

  Number last_output = 0.0;

public:
  using SimpleElement::SimpleElement;

  Setting<Number> initial{0};
  Input<Number> channel{-1};
  Input<Number> control{-1};
  Input<MIDIEvents> input;
  Output<Number> output;
};

//--------------------------------------------------------------------------
// Control Setup
void ControlIn::setup(const SetupContext& context)
{
  SimpleElement::setup(context);
  last_output = initial;
}

//--------------------------------------------------------------------------
// Generate a fragment
void ControlIn::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {}, tie(channel, control, input), tie(output),
                 [&](Number c, Number co, const MIDIEvents& i, Number& o)
  {
    o = last_output;
    for (auto eit = i.rbegin(); eit != i.rend(); ++eit)
    {
      const auto& e = *eit;

      if (e.type == MIDI::Event::Type::control_change &&
          (c < 0 || e.channel == c) && (co < 0 || e.key == co))
        o = last_output = (e.value / 127.0);
    }
  });
}

Dataflow::SimpleModule module
{
  "control-in",
  "ControlIn",
  "midi",
  {
    { "initial",  &ControlIn::initial },
  },
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
