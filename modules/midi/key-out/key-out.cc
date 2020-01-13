//==========================================================================
// MIDI key out module
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../midi-module.h"

namespace {

//==========================================================================
// KeyOut filter
class KeyOut: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  KeyOut *create_clone() const override
  {
    return new KeyOut{module};
  }

public:
  using SimpleElement::SimpleElement;

  Input<Number> channel{-1};
  Input<Number> key{-1};
  Input<Number> velocity{0};
  Input<Trigger> note_on;
  Input<Trigger> note_off;
  Output<MIDI::Event> output;
};

//--------------------------------------------------------------------------
// Generate a fragment
void KeyOut::tick(const TickData& td)
{
  const auto nsamples = td.samples_in_tick(output.get_sample_rate());
  sample_iterate(td, nsamples, {},
                 tie(channel, key, velocity, note_on, note_off),
                 tie(output),
                 [&](Number c, Number co, Number v, Trigger on, Trigger off,
                     MIDI::Event& o)
  {
    if (off)
      o = MIDI::Event(MIDI::Event::Type::note_off, c, co, v * 127.0);
    else if (on)
      o = MIDI::Event(MIDI::Event::Type::note_on, c, co, v * 127.0);
    else
      o = MIDI::Event{};
  });
}

Dataflow::SimpleModule module
{
  "key-out",
  "Key Out",
  "midi",
  {},
  {
    { "channel",  &KeyOut::channel },
    { "key",      &KeyOut::key },
    { "velocity", &KeyOut::velocity },
    { "note-on",  &KeyOut::note_on },
    { "note-off", &KeyOut::note_off },
  },
  {
    { "output",   &KeyOut::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(KeyOut, module)
