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

  double last_velocity{0.0};

public:
  using SimpleElement::SimpleElement;

  Input<double> channel{-1};
  Input<double> key{-1};
  Input<MIDI::Event> input;
  Output<double> velocity;
  Output<double> start;
  Output<double> stop;
};

//--------------------------------------------------------------------------
// Generate a fragment
void KeyIn::tick(const TickData& td)
{
  const auto sample_rate = max(velocity.get_sample_rate(),
                               max(start.get_sample_rate(),
                                   stop.get_sample_rate()));
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(nsamples, {}, tie(channel, key, input),
                 tie(velocity, start, stop),
                 [&](double c, double co, const MIDI::Event& i,
                     double& v, double& _start, double& _stop)
  {
    velocity = last_velocity;
    if (i.type != MIDI::Event::Type::note_on &&
        i.type != MIDI::Event::Type::note_off)
      return;
    if ((c < 0 || i.channel == c) && (co < 0 || i.key == co))
    {
      v = last_velocity = i.value / 127.0;
      _start = i.type == MIDI::Event::Type::note_on;
      _stop = i.type == MIDI::Event::Type::note_off;
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
    { "start",    &KeyIn::start },
    { "stop",     &KeyIn::stop },
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(KeyIn, module)
