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

  enum class Slope
  {
    free,
    rising,
    falling
  } slope = Slope::free;

  // Filter/Element virtuals
  void accept(FragmentPtr fragment) override;

public:
  double level{0};
  int points{0};  // If 0, whatever is in the waveform

  using FragmentFilter::FragmentFilter;

  // Getters/Setters
  string get_mode() const;
  void set_mode(const string& mode);
  string get_slope() const;
  void set_slope(const string& slope);
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
  return "";
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
// Get slope
string AudioToVectorFilter::get_slope() const
{
  switch (slope)
  {
    case Slope::free: return "free";
    case Slope::rising: return "rising";
    case Slope::falling: return "falling";
  }
  return "";
}

//--------------------------------------------------------------------------
// Set slope
void AudioToVectorFilter::set_slope(const string& m)
{
  if (m == "free")
    slope = Slope::free;
  else if (m == "rising")
    slope = Slope::rising;
  else if (m == "falling")
    slope = Slope::falling;
  else
  {
    Log::Error log;
    log << "Unknown slope '" << m << "' in " << id << endl;
  }
}

//--------------------------------------------------------------------------
// Process some data
void AudioToVectorFilter::accept(FragmentPtr fragment)
{
  FramePtr frame{new Frame{fragment->timestamp}};
  auto c = 0u;
  auto s = 0u;
  const auto channels = fragment->waveforms.size();
  for (const auto& wit: fragment->waveforms)
  {
    const auto& w = wit.second;
    auto npoints = w.size();
    auto start_point = 0u;

    // Find start point from trigger slope
    switch (slope)
    {
      case Slope::free:
        // Use all of it
        break;

      case Slope::rising:
        for (auto i = 0u; i < npoints; ++i)
        {
          if (i && w[i-1] < level && w[i] >= level)
          {
            start_point = i;
            break;
          }
        }
        break;

      case Slope::falling:
        for (auto i = 0u; i < npoints; ++i)
        {
          if (i && w[i-1] > level && w[i] <= level)
          {
            start_point = i;
            break;
          }
        }
        break;
    }

    npoints -= start_point;

    // Limit to max points - this remove trailing flicker when
    // using triggering
    if (points && points < (int)npoints) npoints = points;

    if (mode == Mode::multi)
      frame->points.resize(frame->points.size() + npoints);
    else
      frame->points.resize(max(frame->points.size(), npoints));

    for (auto i = 0u; i < npoints; ++i, ++s)
    {
      const auto a = max(-1.0f, min(1.0f, w[start_point+i]));

      switch (mode)
      {
        case Mode::multi:
          frame->points[s] = {static_cast<double>(i) / npoints - 0.5,
                              ((a + 1.0) / 2) / channels
                                - static_cast<double>(c + 1) / channels,
                              i ? Colour::white : Colour::black};
          break;
        case Mode::combined:
          if (c)
            frame->points[i].y += (a / channels) / 2;
          else
            frame->points[i] = {static_cast<double>(i) / npoints - 0.5,
                                (a / channels) / 2,
                                i ? Colour::white : Colour::black};
          break;
        case Mode::first:
          frame->points[i] = {static_cast<double>(i) / npoints - 0.5,
                              a / 2, i ? Colour::white : Colour::black};
          break;
      }
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
  "Convert audio data to a vector waveform",
  "audio",
  {
    { "mode", { "Mode to run in", Value::Type::choice,
                { &AudioToVectorFilter::get_mode,
                  &AudioToVectorFilter::set_mode },
                { "multi", "combined", "first" }, false } },
    { "slope", { "Trigger slope", Value::Type::choice,
                { &AudioToVectorFilter::get_slope,
                  &AudioToVectorFilter::set_slope },
                { "free", "rising", "falling" }, true } },
    { "level", { "Trigger level (0-1)", Value::Type::number,
                 &AudioToVectorFilter::level, true } },
    { "points", { "Maximum points (0 = unlimited)", Value::Type::number,
                 &AudioToVectorFilter::points, true } }
  },
  { "Audio" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AudioToVectorFilter, module)
