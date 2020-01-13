//==========================================================================
// ViGraph dataflow module: core/key-in/key-in.cc
//
// MIDI key in module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../midi-module.h"

namespace {

//==========================================================================
// KeyIn filter
class KeyIn: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  KeyIn *create_clone() const override
  {
    return new KeyIn{module};
  }

  Number last_velocity{0.0};

public:
  using SimpleElement::SimpleElement;

  Input<Number> channel{-1};
  Input<Number> key{-1};
  Input<MIDI::Event> input;
  Output<Number> velocity;
  Output<Trigger> note_on;
  Output<Trigger> note_off;
};

//--------------------------------------------------------------------------
// Generate a fragment
void KeyIn::tick(const TickData& td)
{
  const auto sample_rate = max(velocity.get_sample_rate(),
                               max(note_on.get_sample_rate(),
                                   note_off.get_sample_rate()));
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, tie(channel, key, input),
                 tie(velocity, note_on, note_off),
                 [&](Number c, Number co, const MIDI::Event& i,
                     Number& v, Trigger& on, Trigger& off)
  {
    v = last_velocity;
    on = 0;
    off = 0;
    if (i.type != MIDI::Event::Type::note_on &&
        i.type != MIDI::Event::Type::note_off)
      return;
    if ((c < 0 || i.channel == c) && (co < 0 || i.key == co))
    {
      v = last_velocity = i.value / 127.0;
      on = i.type == MIDI::Event::Type::note_on;
      off = i.type == MIDI::Event::Type::note_off;
    }
  });
}

Dataflow::SimpleModule module
{
  "key-in",
  "MIDI Key In",
  "midi",
  {},
  {
    { "channel",  &KeyIn::channel },
    { "key",      &KeyIn::key },
    { "input",    &KeyIn::input },
  },
  {
    { "velocity", &KeyIn::velocity },
    { "note-on",  &KeyIn::note_on },
    { "note-off", &KeyIn::note_off },
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(KeyIn, module)
