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
  void setup() override;
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
    timestamp_t last_used = 0;
    Voice(int _voice, timestamp_t _last_used):
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

  Setting<int> voices;
  Input<double> channel{-1};
  Input<MIDI::Event> input;
  Output<MIDI::Event> output;
};

//--------------------------------------------------------------------------
// Setup
void AssignVoice::setup()
{
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
    available.emplace(i, 0);
  }
}

//--------------------------------------------------------------------------
// Tick
void AssignVoice::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();
  auto t = td.first_sample_at(sample_rate);
  const auto sample_duration = td.sample_duration(sample_rate);
  const auto nsamples = td.samples_in_tick(sample_rate);
  sample_iterate(nsamples, {}, tie(channel, input), tie(output),
                 [&](double c, const MIDI::Event& i, MIDI::Event& o)
  {
    o = i;
    if (c == i.channel)
    {
      switch (i.type)
      {
        case MIDI::Event::Type::note_on:
          {
            auto it = key_to_voice.find(i.key);
            if (it != key_to_voice.end())
            {
              o.voice = it->second;
            }
            else if (!available.empty())
            {
              auto at = available.begin();
              o.voice = at->voice;
              available.erase(at);
              key_to_voice.emplace(i.key, o.voice);
            }
          }
          break;
        case MIDI::Event::Type::note_off:
          {
            auto it = key_to_voice.find(i.key);
            if (it != key_to_voice.end())
            {
              o.voice = it->second;
              key_to_voice.erase(it);
              available.emplace(o.voice, t);
            }
          }
          break;
        default:
          break;
      }
    }
    t += sample_duration;
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
