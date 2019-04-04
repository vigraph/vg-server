//==========================================================================
// ViGraph dataflow module:
//    audio/filters/loop/loop.cc
//
// Audio loop filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"

namespace {

using namespace ViGraph::Dataflow;

//==========================================================================
// Loop filter
class LoopFilter: public FragmentFilter
{
  // 3 sets of per channel buffers
  vector<map<Speaker, vector<sample_t>>>
    buffers = vector<map<Speaker, vector<sample_t>>>(3);
  unsigned long playback_pos = 0;

  const unsigned playback_buffer = 0;
  const unsigned recording_buffer = 1;
  const unsigned recorded_buffer = 2;

  bool recording = false;
  bool playing = false;
  bool new_recording_ready = false;

  // Source/Element virtuals
  void accept(FragmentPtr fragment) override;
  void tick(const TickData& td) override;

public:
  using FragmentFilter::FragmentFilter;

  void start_playback();
  void stop_playback() { playing = false; }
  void start_recording();
  void stop_recording();
};

//--------------------------------------------------------------------------
// Start playback
void LoopFilter::start_playback()
{
  if (!playing)
  {
    buffers[playback_buffer] = buffers[recorded_buffer];
    playing = true;
    playback_pos = 0;
    new_recording_ready = false;
  }
}

//--------------------------------------------------------------------------
// Start recording
void LoopFilter::start_recording()
{
  if (!recording)
  {
    buffers[recording_buffer].clear();
    recording = true;
  }
}

//--------------------------------------------------------------------------
// Stop recording
void LoopFilter::stop_recording()
{
  if (recording)
  {
    buffers[recorded_buffer] = buffers[recording_buffer];
    recording = false;
    new_recording_ready = true;
  }
}

//--------------------------------------------------------------------------
// Process some data
void LoopFilter::accept(FragmentPtr fragment)
{
  if (recording)
  {
    auto& buffer = buffers[recording_buffer];

    for (const auto& wit: fragment->waveforms)
    {
      const auto c = wit.first;
      const auto& w = wit.second;

      auto bit = buffer.find(c);
      if (bit == buffer.end())
        bit = buffer.emplace(c, vector<sample_t>{}).first;

      auto &b = bit->second;
      b.reserve(b.size() + w.size());
      for (auto i = 0u; i < w.size(); ++i)
        b.emplace_back(w[i]);
    }
  }
}

//--------------------------------------------------------------------------
// Generate a fragment
void LoopFilter::tick(const TickData& td)
{
  if (playing)
  {
    auto nsamples = td.samples(sample_rate);
    auto& buffer = buffers[playback_buffer];

    auto fragment = new Fragment(td.t);
    for (const auto& bit: buffer)
    {
      const auto c = bit.first;
      fragment->waveforms[c].reserve(nsamples);
    }

    while (nsamples)
    {
      if (buffer.empty() || buffer.begin()->second.empty())
        break;
      auto chunk_size = 0;
      auto looped = false;
      for (const auto& bit: buffer)
      {
        const auto c = bit.first;
        const auto& w = bit.second;
        if (!chunk_size)
        {
          if (playback_pos + nsamples > w.size())
          {
            chunk_size = w.size() - playback_pos;
            looped = true;
          }
          else
          {
            chunk_size = nsamples;
          }
        }
        copy(&w[playback_pos], &w[min(playback_pos + chunk_size, w.size())],
             back_inserter(fragment->waveforms[c]));
      }
      nsamples -= chunk_size;
      playback_pos += chunk_size;

      if (looped)
      {
        if (new_recording_ready)
        {
          buffer = buffers[recorded_buffer];
          new_recording_ready = false;
        }
        playback_pos = 0;
      }
    }
    send(fragment);
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "loop",
  "Audio loop",
  "Audio loop",
  "audio",
  {
    { "start-playback", { "Start playback", Value::Type::trigger,
                          &LoopFilter::start_playback, true } },
    { "stop-playback", { "Stop playback", Value::Type::trigger,
                         &LoopFilter::stop_playback, true } },
    { "start-recording", { "Start recording", Value::Type::trigger,
                           &LoopFilter::start_recording, true } },
    { "stop-recording", { "Stop recording", Value::Type::trigger,
                          &LoopFilter::stop_recording, true } },
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(LoopFilter, module)
