//==========================================================================
// ViGraph dataflow module: audio/loop/loop.cc
//
// Loop module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../audio-module.h"

namespace {

//==========================================================================
// Loop
class Loop: public SimpleElement
{
private:
  bool recording = false;
  bool playing = false;
  bool recorded_ready = false;
  vector<double> recording_buffer;
  vector<double> recorded_buffer;
  vector<double> play_buffer;
  uint64_t play_pos = 0;

  // Element virtuals
  void tick(const TickData& td) override;

  // Clone
  Loop *create_clone() const override
  {
    return new Loop{module};
  }

public:
  using SimpleElement::SimpleElement;

  // Configuration
  Input<double> input{0.0};
  Input<Trigger> play_start{0.0};
  Input<Trigger> play_stop{0.0};
  Input<Trigger> record_start{0.0};
  Input<Trigger> record_stop{0.0};
  Output<double> output;
};

//--------------------------------------------------------------------------
// Tick data
void Loop::tick(const TickData& td)
{
  const auto sample_rate = output.get_sample_rate();

  const auto nsamples = td.samples_in_tick(sample_rate);

  sample_iterate(td, nsamples, {}, tie(input, play_start, play_stop,
                                   record_start, record_stop), tie(output),
                 [&](double i, Trigger pb, Trigger pe,
                     Trigger rb, Trigger re,
                     double& o)
  {
    if (recording)
    {
      if (re)
      {
        recording = false;
        recorded_buffer = recording_buffer;
        recorded_ready = true;
      }
      if (rb)
      {
        recording = true;
        recording_buffer.clear();
      }
    }
    else
    {
      if (rb)
      {
        recording = true;
        recording_buffer.clear();
      }
      if (re)
      {
        recording = false;
        recorded_buffer = recording_buffer;
        recorded_ready = true;
      }
    }

    if (playing)
    {
      if (pe)
      {
        playing = false;
      }
      if (pb)
      {
        playing = true;
        play_buffer = recorded_buffer;
        recorded_ready = false;
        play_pos = 0;
      }
    }
    else
    {
      if (pb)
      {
        playing = true;
        play_buffer = recorded_buffer;
        recorded_ready = false;
        play_pos = 0;
      }
      if (pe)
      {
        playing = false;
      }
    }

    if (recording)
      recording_buffer.push_back(i);

    if (playing)
    {
      o = play_buffer[play_pos++];
      if (play_pos > play_buffer.size())
      {
        if (recorded_ready)
        {
          play_buffer = recorded_buffer;
          recorded_ready = false;
        }
        play_pos = 0;
      }
    }
    else
    {
      o = 0;
    }
  });
}

Dataflow::SimpleModule module
{
  "loop",
  "Loop",
  "audio",
  {},
  {
    { "input",        &Loop::input },
    { "play-start",   &Loop::play_start },
    { "play-stop",    &Loop::play_stop },
    { "record-start", &Loop::record_start },
    { "record-stop",  &Loop::record_stop },
  },
  {
    { "output",       &Loop::output }
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(Loop, module)
