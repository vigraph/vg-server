//==========================================================================
// MIDI button in module
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../midi-module.h"

namespace {

//==========================================================================
// ButtonIn filter
class ButtonIn: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  ButtonIn *create_clone() const override
  {
    return new ButtonIn{module};
  }

  Number last_value = 0.0;

public:
  using SimpleElement::SimpleElement;

  Input<Number> channel{-1};
  Input<Number> control{-1};
  Input<MIDIEvents> input;
  Output<Trigger> pressed;
  Output<Trigger> released;
};

//--------------------------------------------------------------------------
// Generate a fragment
void ButtonIn::tick(const TickData& td)
{
  const auto sample_rate = max(pressed.get_sample_rate(),
                               released.get_sample_rate());
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, tie(channel, control, input),
                 tie(pressed, released),
                 [&](Number c, Number co, const MIDIEvents& i,
                     Trigger& p, Trigger& r)
  {
    p = r = 0;
    for (auto eit = i.rbegin(); eit != i.rend(); ++eit)
    {
      const auto& e = *eit;
      if (e.type == MIDI::Event::Type::control_change &&
          (c < 0 || e.channel == c) && (co < 0 || e.key == co))
      {
        if (!last_value && e.value)
          p = 1;
        else if (last_value && !e.value)
          r = 1;
        last_value = e.value;
        break;
      }
    }
  });
}

Dataflow::SimpleModule module
{
  "button-in",
  "ButtonIn",
  "midi",
  {},
  {
    { "channel",  &ButtonIn::channel },
    { "control",  &ButtonIn::control },
    { "input",    &ButtonIn::input },
  },
  {
    { "pressed",  &ButtonIn::pressed },
    { "released", &ButtonIn::released },
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ButtonIn, module)
