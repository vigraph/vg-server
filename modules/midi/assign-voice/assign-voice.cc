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

  vector<int> voice_map;


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
  voice_map = vector<int>(voices, -1);
}

//--------------------------------------------------------------------------
// Tick
void AssignVoice::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();
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
            auto v = 0;
            for (auto j = 0u; j < voice_map.size(); ++j)
            {
              if (voice_map[j] == i.key)
              {
                v = j + 1;
                break;
              }
            }
            if (!v)
            {
              for (auto j = 0u; j < voice_map.size(); ++j)
              {
                if (voice_map[j] == -1)
                {
                  v = j + 1;
                  voice_map[j] = i.key;
                  break;
                }
              }
            }
            o.voice = v;
          }
          break;
        case MIDI::Event::Type::note_off:
          for (auto j = 0u; j < voice_map.size(); ++j)
          {
            if (voice_map[j] == i.key)
            {
              o.voice = j + 1;
              voice_map[j] = -1;
              break;
            }
          }
          break;
        default:
          break;
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
