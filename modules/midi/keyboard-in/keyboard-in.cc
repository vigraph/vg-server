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

  Number last_note{0.0};
  Number last_velocity{0.0};

public:
  using SimpleElement::SimpleElement;

  Input<Number> channel{-1};
  Input<Number> voice{-1};
  Input<MIDI::Event> input;
  Output<Number> note;
  Output<Number> velocity;
  Output<Trigger> pressed;
  Output<Trigger> released;
};

//--------------------------------------------------------------------------
// Generate a fragment
void KeyboardIn::tick(const TickData& td)
{
  const auto sample_rate = max(note.get_sample_rate(),
                               max(velocity.get_sample_rate(),
                                   max(pressed.get_sample_rate(),
                                       released.get_sample_rate())));
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, tie(channel, voice, input),
                 tie(note, velocity, pressed, released),
                 [&](Number channel, Number voice, const MIDI::Event& input,
                     Number& note, Number& velocity,
                     Trigger& pressed, Trigger& released)
  {
    note = last_note;
    velocity = last_velocity;
    pressed = 0;
    released = 0;
    if (input.type != MIDI::Event::Type::note_on &&
        input.type != MIDI::Event::Type::note_off)
      return;
    if ((channel < 0 || input.channel == channel)
        && (voice < 0 || input.voice == voice))
    {
      note = last_note = MIDI::get_midi_cv(input.key);
      if (input.type == MIDI::Event::Type::note_on)
        velocity = last_velocity = input.value / 127.0;
      pressed = input.type == MIDI::Event::Type::note_on;
      released = input.type == MIDI::Event::Type::note_off;
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
    { "voice",      &KeyboardIn::voice   },
    { "input",      &KeyboardIn::input   },
  },
  {
    { "note",       &KeyboardIn::note     },
    { "velocity",   &KeyboardIn::velocity },
    { "pressed",    &KeyboardIn::pressed  },
    { "released",   &KeyboardIn::released },
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(KeyboardIn, module)
