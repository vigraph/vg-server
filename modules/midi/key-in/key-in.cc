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

public:
  using SimpleElement::SimpleElement;

  Input<Number> channel{-1};
  Input<Number> min{-1};
  Input<Number> max{-1};
  Input<MIDI::Event> input;
  Output<Number> key;
  Output<Number> velocity;
  Output<Trigger> note_on;
  Output<Trigger> note_off;
};

//--------------------------------------------------------------------------
// Generate a fragment
void KeyIn::tick(const TickData& td)
{
  const auto sample_rate = std::max(key.get_sample_rate(),
                                    std::max(velocity.get_sample_rate(),
                                      std::max(note_on.get_sample_rate(),
                                               note_off.get_sample_rate())));
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, tie(channel, min, max, input),
                 tie(key, velocity, note_on, note_off),
                 [&](Number c, Number min, Number max, const MIDI::Event& i,
                     Number& k, Number& v, Trigger& on, Trigger& off)
  {
    k = 0;
    v = 0;
    on = 0;
    off = 0;
    if (i.type != MIDI::Event::Type::note_on &&
        i.type != MIDI::Event::Type::note_off)
      return;
    if ((c < 0 || i.channel == c) &&
        (min < 0 || i.key >= min) &&
        (max < 0 || i.key <= max))
    {
      k = i.key;
      v = i.value / 127.0;
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
    { "min",      &KeyIn::min },
    { "max",      &KeyIn::max },
    { "input",    &KeyIn::input },
  },
  {
    { "key",      &KeyIn::key },
    { "velocity", &KeyIn::velocity },
    { "note-on",  &KeyIn::note_on },
    { "note-off", &KeyIn::note_off },
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(KeyIn, module)
