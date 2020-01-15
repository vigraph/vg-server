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

  queue<MIDIEvent> buffer;

public:
  using SimpleElement::SimpleElement;

  Input<Number> channel{-1};
  Input<Number> min{-1};
  Input<Number> max{-1};
  Input<MIDIEvents> input;
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
                 [&](Number c, Number min, Number max, const MIDIEvents& i,
                     Number& k, Number& v, Trigger& on, Trigger& off)
  {
    k = 0;
    v = 0;
    on = 0;
    off = 0;
    bool sent = false;
    if (!buffer.empty())
    {
      const auto& e = buffer.front();
      k = e.key;
      v = e.value / 127.0;
      on = e.type == MIDI::Event::Type::note_on;
      off = e.type == MIDI::Event::Type::note_off;
      buffer.pop();
      sent = true;
    }
    for (const auto& e: i)
    {
      if (e.type != MIDI::Event::Type::note_on &&
          e.type != MIDI::Event::Type::note_off)
        continue;
      if ((c < 0 || e.channel == c) &&
          (min < 0 || e.key >= min) &&
          (max < 0 || e.key <= max))
      {
        if (sent)
        {
          buffer.emplace(e);
        }
        else
        {
          k = e.key;
          v = e.value / 127.0;
          on = e.type == MIDI::Event::Type::note_on;
          off = e.type == MIDI::Event::Type::note_off;
          sent = true;
        }
        break;
      }
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
