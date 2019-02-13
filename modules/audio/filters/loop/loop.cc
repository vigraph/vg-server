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
  vector<vector<vector<sample_t>>> buffers; // Buffer of buffers per channel
  unsigned playback_pos = 0;

  unsigned playback_buffer = 0;
  unsigned recording_buffer = 1;
  unsigned recorded_buffer = 2;

  bool recording = false;
  bool playing = false;
  bool new_recording_ready = false;

  // Source/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FragmentPtr fragment) override;
  void tick(const TickData& td) override;

public:
  LoopFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <loop />
LoopFilter::LoopFilter(const Dataflow::Module *module,
                         const XML::Element& config):
    Element(module, config), FragmentFilter(module, config)
{
  buffers.resize(3);
}

//--------------------------------------------------------------------------
// Set a control property
void LoopFilter::set_property(const string& property, const SetParams&)
{
  if (property == "start")
  {
    if (!playing)
    {
      buffers[playback_buffer] = buffers[recorded_buffer];
      playing = true;
      playback_pos = 0;
      new_recording_ready = false;
    }
  }
  else if (property == "stop")
  {
    playing = false;
  }
  else if (property == "trigger")
  {
    if (!recording)
    {
      buffers[recording_buffer].clear();
      recording = true;
    }
  }
  else if (property == "clear")
  {
    if (recording)
    {
      buffers[recorded_buffer] = buffers[recording_buffer];
      recording = false;
      new_recording_ready = true;
    }
  }
}

//--------------------------------------------------------------------------
// Process some data
void LoopFilter::accept(FragmentPtr fragment)
{
  if (recording)
  {
    auto& buffer = buffers[recording_buffer];
    if (fragment->nchannels > buffer.size())
    {
      buffer.resize(fragment->nchannels);
      auto s = 0u;
      for (auto& b: buffer)
      {
        if (!s)
          s = b.size();
        b.resize(s);
      }
    }
    for (auto i = 0u; i < fragment->waveform.size() / fragment->nchannels; ++i)
    {
      for (auto c = 0u; c < buffer.size(); ++c)
      {
        if (c < fragment->nchannels)
        {
          const auto s = fragment->waveform[i * fragment->nchannels + c];
          buffer[c].emplace_back(s);
        }
        else
        {
          buffer[c].emplace_back(0.0);
        }
      }
    }

  }
}

//--------------------------------------------------------------------------
// Generate a fragment
void LoopFilter::tick(const TickData& td)
{
  if (playing)
  {
    const auto nsamples = td.samples(sample_rate);
    auto& buffer = buffers[playback_buffer];
    auto channels = buffer.size();

    if (!channels)
    {
      if (new_recording_ready)
      {
        buffer = buffers[recorded_buffer];
        new_recording_ready = false;
        playback_pos = 0;
        channels = buffer.size();
      }
    }

    if (channels)
    {
      auto samples = buffer.front().size();

      if (!samples)
      {
        if (new_recording_ready)
        {
          buffer = buffers[recorded_buffer];
          new_recording_ready = false;
          playback_pos = 0;
          channels = buffer.size();
          if (channels)
            samples = buffer.front().size();
        }
      }

      if (samples)
      {
        auto fragment = new Fragment(td.t, channels);
        fragment->waveform.reserve(nsamples * channels);

        for (auto i = 0u; i < nsamples; ++i, ++playback_pos)
        {
          if (playback_pos > samples)
          {
            if (new_recording_ready)
            {
              buffer = buffers[recorded_buffer];
              new_recording_ready = false;
              playback_pos = 0;
              channels = buffer.size();
              if (!channels)
                break;
              samples = buffer.front().size();
              if (!samples)
                break;
            }
            playback_pos = 0;
          }

          for (auto c = 0u; c < channels; ++c)
            fragment->waveform.push_back(buffer[c][playback_pos]);
        }
        send(fragment);
      }
    }
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
    { "start", { "Start playback", Value::Type::trigger, "", true } },
    { "stop", { "Stop playback", Value::Type::trigger, "", true } },
    { "trigger", { "Start recording", Value::Type::trigger, "", true } },
    { "clear", { "Stop recording", Value::Type::trigger, "", true } },
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(LoopFilter, module)
