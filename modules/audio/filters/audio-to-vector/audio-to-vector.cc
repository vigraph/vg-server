//==========================================================================
// ViGraph dataflow module:
//    audio/filters/audio-to-vector/audio-to-vector.cc
//
// Audio audio-to-vector filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"
#include "../../../vector/vector-module.h"

namespace {

using namespace ViGraph::Dataflow;
using namespace ViGraph::Module;

//==========================================================================
// VU meter filter
class AudioToVectorFilter: public FragmentFilter
{
private:
  enum class Mode
  {
    multi,
    combined,
    first
  } mode = Mode::multi;

  // Filter/Element virtuals
  void accept(FragmentPtr fragment) override;

public:
  using FragmentFilter::FragmentFilter;

  // Getters/Setters
  string get_mode() const;
  void set_mode(const string& mode);
};

//--------------------------------------------------------------------------
// Get mode
string AudioToVectorFilter::get_mode() const
{
  switch (mode)
  {
    case Mode::multi: return "multi";
    case Mode::combined: return "combined";
    case Mode::first: return "first";
  }
}

//--------------------------------------------------------------------------
// Set mode
void AudioToVectorFilter::set_mode(const string& m)
{
  if (m == "multi")
    mode = Mode::multi;
  else if (m == "combined")
    mode = Mode::combined;
  else if (m == "first")
    mode = Mode::first;
  else
  {
    Log::Error log;
    log << "Unknown mode '" << m << "' in " << id << endl;
  }
}

//--------------------------------------------------------------------------
// Process some data
void AudioToVectorFilter::accept(FragmentPtr fragment)
{
  FramePtr frame{new Frame{0}};
  auto c = 0u;
  auto s = 0u;
  const auto channels = fragment->waveforms.size();
  for (const auto& wit: fragment->waveforms)
  {
    const auto& w = wit.second;
    if (mode == Mode::multi)
      frame->points.resize(frame->points.size() + w.size());
    else
      frame->points.resize(max(frame->points.size(), w.size()));
    for (auto i = 0u; i < w.size(); ++i)
    {
      const auto a = max(-1.0f, min(1.0f, w[i]));
      switch (mode)
      {
        case Mode::multi:
          frame->points[s] = {static_cast<double>(i) / w.size() - 0.5,
                              ((a + 1.0) / 2) / channels
                                - static_cast<double>(c + 1) / channels,
                              i ? Colour::white : Colour::black};
          break;
        case Mode::combined:
          if (c)
            frame->points[i].y += (a / channels) / 2;
          else
            frame->points[i] = {static_cast<double>(i) / w.size() - 0.5,
                                (a / channels) / 2,
                                i ? Colour::white : Colour::black};
          break;
        case Mode::first:
          frame->points[i] = {static_cast<double>(i) / w.size() - 0.5,
                              a / 2, i ? Colour::white : Colour::black};
          break;
      }
      ++s;
    }
    if (mode == Mode::first)
      break;
    ++c;
  }
  if (!frame->points.empty())
    send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "audio-to-vector",
  "Audio-to-Vector",
  "Send copy of audio data to vector data via router",
  "audio",
  {
    { "mode", { "Mode to run in", Value::Type::choice,
                { &AudioToVectorFilter::get_mode,
                  &AudioToVectorFilter::set_mode },
                { "multi", "combined", "first" }, false } },
  },
  { "Audio" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AudioToVectorFilter, module)
