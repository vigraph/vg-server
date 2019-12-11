//==========================================================================
// ViGraph dataflow module: core/keyboard-in/keyboard-in.cc
//
// MIDI keyboard in module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../midi-module.h"

namespace {

//==========================================================================
// KeyboardIn filter
class KeyboardIn: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  KeyboardIn *create_clone() const override
  {
    return new KeyboardIn{module};
  }

  Number last_frequency{0.0};
  Number last_velocity{0.0};

public:
  using SimpleElement::SimpleElement;

  Input<Number> channel{-1};
  Input<Number> voice{-1};
  Input<MIDI::Event> input;
  Output<Number> frequency;
  Output<Number> velocity;
  Output<Trigger> start;
  Output<Trigger> stop;
};

//--------------------------------------------------------------------------
// Generate a fragment
void KeyboardIn::tick(const TickData& td)
{
  const auto sample_rate = max(frequency.get_sample_rate(),
                               max(velocity.get_sample_rate(),
                                   max(start.get_sample_rate(),
                                       stop.get_sample_rate())));
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, tie(channel, voice, input),
                 tie(frequency, velocity, start, stop),
                 [&](Number c, Number voice, const MIDI::Event& i,
                     Number& f, Number& v, Trigger& _start, Trigger& _stop)
  {
    f = last_frequency;
    v = last_velocity;
    _start = 0;
    _stop = 0;
    if (i.type != MIDI::Event::Type::note_on &&
        i.type != MIDI::Event::Type::note_off)
      return;
    if ((c < 0 || i.channel == c) && (voice < 0 || i.voice == voice))
    {
      f = last_frequency = MIDI::get_midi_frequency(i.key);
      if (i.type == MIDI::Event::Type::note_on)
        v = last_velocity = i.value / 127.0;
      _start = i.type == MIDI::Event::Type::note_on;
      _stop = i.type == MIDI::Event::Type::note_off;
    }
  });
}

Dataflow::SimpleModule module
{
  "keyboard-in",
  "MIDI Keyboard In",
  "midi",
  {},
  {
    { "channel",    &KeyboardIn::channel },
    { "voice",      &KeyboardIn::voice },
    { "input",      &KeyboardIn::input },
  },
  {
    { "frequency",  &KeyboardIn::frequency },
    { "velocity",   &KeyboardIn::velocity },
    { "start",      &KeyboardIn::start },
    { "stop",       &KeyboardIn::stop },
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(KeyboardIn, module)
