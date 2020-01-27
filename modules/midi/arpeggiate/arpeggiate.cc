//==========================================================================
// MIDI arpeggiate module
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../midi-module.h"

namespace {

//==========================================================================
// Arpeggiate filter
class Arpeggiate: public SimpleElement
{
private:
  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Arpeggiate *create_clone() const override
  {
    return new Arpeggiate{module};
  }

  struct Event: public MIDIEvent
  {
    bool release = false;

    Event(const MIDIEvent& me): MIDIEvent(me) {}
  };
  vector<Event> events;
  int current = -1;
  bool holding = false;

  // Release marked events
  void release_events(MIDIEvents& o);

public:
  using SimpleElement::SimpleElement;

  Input<Number> channel{-1};
  Input<Trigger> next;
  Input<Trigger> hold;
  Input<Trigger> release;
  Input<MIDIEvents> input;
  Output<MIDIEvents> output;
};

//--------------------------------------------------------------------------
// Tick
void Arpeggiate::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, tie(channel, next, hold, release, input),
                 tie(output),
                 [&](Number c, Trigger n, Trigger h, Trigger r,
                     const MIDIEvents& i, MIDIEvents& o)
  {
    o = MIDIEvents{};
    if (holding)
    {
      if (r)
      {
        holding = false;
        release_events(o);
      }
      if (h)
        holding = true;
    }
    else
    {
      if (h)
        holding = true;
      if (r)
      {
        holding = false;
        release_events(o);
      }
    }
    for (const auto e: i)
    {
      if (e.channel == c)
      {
        if (e.type == MIDI::Event::Type::note_on)
        {
          events.emplace_back(e);
        }
        else if (e.type == MIDI::Event::Type::note_off)
        {
          for (auto& qe: events)
          {
            if (qe.channel == e.channel && qe.key == e.key)
            {
              qe.release = true;
            }
          }
        }
      }
    }
    if (n && !events.empty())
    {
      if (current >= 0)
      {
        o.emplace_back(events[current]);
        o.back().type = MIDI::Event::Type::note_off;
        if (!holding)
        {
          while (events[current].release)
          {
            events.erase(events.begin() + current);
            if (events.empty())
            {
              current = -1;
              break;
            }
            if (current >= static_cast<int>(events.size()))
              current = 0;
          }
        }
      }
      if (!events.empty())
      {
        if (++current >= static_cast<int>(events.size()))
          current = 0;
        o.emplace_back(events[current]);
      }
    }
  });
}

//--------------------------------------------------------------------------
// Release held events
void Arpeggiate::release_events(MIDIEvents&)
{
}

Dataflow::SimpleModule module
{
  "arpeggiate",
  "MIDI Arpeggiator",
  "midi",
  {},
  {
    { "channel",    &Arpeggiate::channel },
    { "input",      &Arpeggiate::input   },
    { "next",       &Arpeggiate::next    },
    { "hold",       &Arpeggiate::hold    },
    { "release",    &Arpeggiate::release },
  },
  {
    { "output",     &Arpeggiate::output  },
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Arpeggiate, module)
