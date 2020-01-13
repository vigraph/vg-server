//==========================================================================
// ViGraph dataflow module: core/assign-voice/assign-voice.cc
//
// MIDI key in module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../midi-module.h"

namespace {

//==========================================================================
// AssignVoice filter
class AssignVoice: public SimpleElement
{
private:
  // Element virtuals
  void setup(const SetupContext& context) override;
  void tick(const TickData& td) override;

  // Clone
  AssignVoice *create_clone() const override
  {
    return new AssignVoice{module};
  }

  map<int, int> key_to_voice;
  struct Voice
  {
    int voice = -1;
    Time::Duration last_used;
    Voice(int _voice, const Time::Duration& _last_used):
      voice{_voice}, last_used{_last_used}
    {}
    bool operator<(const Voice& b) const
    {
      if (last_used == b.last_used)
        return voice < b.voice;
      return last_used < b.last_used;
    }
  };
  std::set<Voice> available;

public:
  using SimpleElement::SimpleElement;

  Setting<Integer> voices;
  Input<Number> channel{-1};
  Input<MIDIEvents> input;
  Output<MIDIEvents> output;
};

//--------------------------------------------------------------------------
// Setup
void AssignVoice::setup(const SetupContext& context)
{
  SimpleElement::setup(context);

  auto max = 0;

  // Remove entries higher than number of voices
  for (auto it = key_to_voice.begin(); it != key_to_voice.end();)
  {
    if (it->second > voices)
    {
      it = key_to_voice.erase(it);
    }
    else
    {
      if (it->second > max)
        max = it->second;
      ++it;
    }
  }
  for (auto it = available.begin(); it != available.end();)
  {
    if (it->voice > voices)
    {
      it = available.erase(it);
    }
    else
    {
      if (it->voice > max)
        max = it->voice;
      ++it;
    }
  }

  // Add new voices to available pool
  for (auto i = max + 1; i < voices; ++i)
  {
    available.emplace(i, Time::Duration{});
  }
}

//--------------------------------------------------------------------------
// Tick
void AssignVoice::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(td, nsamples, {}, tie(channel, input), tie(output),
                 [&](Number c, const MIDIEvents& i, MIDIEvents& o)
  {
    o = i;
    for (auto& e: o)
    {
      if (c == e.channel)
      {
        switch (e.type)
        {
          case MIDI::Event::Type::note_on:
            {
              auto it = key_to_voice.find(e.key);
              if (it != key_to_voice.end())
              {
                e.voice = it->second;
              }
              else if (!available.empty())
              {
                auto at = available.begin();
                e.voice = at->voice;
                available.erase(at);
                key_to_voice.emplace(e.key, e.voice);
              }
            }
            break;
          case MIDI::Event::Type::note_off:
            {
              auto it = key_to_voice.find(e.key);
              if (it != key_to_voice.end())
              {
                e.voice = it->second;
                key_to_voice.erase(it);
                available.emplace(e.voice, e.time);
              }
            }
            break;
          default:
            break;
        }
      }
    }
  });
}

Dataflow::SimpleModule module
{
  "assign-voice",
  "MIDI voice assigner",
  "midi",
  {
    { "voices",   &AssignVoice::voices },
  },
  {
    { "channel",  &AssignVoice::channel },
    { "input",    &AssignVoice::input },
  },
  {
    { "output",   &AssignVoice::output },
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AssignVoice, module)
