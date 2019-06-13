//==========================================================================
// ViGraph dataflow module:
//    audio/filters/attenuator/attenuator.cc
//
// Audio attenuator filter
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"
#include <algorithm>

namespace {

using namespace ViGraph::Dataflow;

//==========================================================================
// Attenuator filter
class AttenuatorFilter: public FragmentFilter
{
private:
  vector<double> gain{1.0};

  // Source/Element virtuals
  void accept(FragmentPtr fragment) override;

public:
  using FragmentFilter::FragmentFilter;

  // Getters/Setters
  double get_gain() const { return gain.back(); }
  void set_gain(const vector<double>& g);
};

//--------------------------------------------------------------------------
// Set gain
void AttenuatorFilter::set_gain(const vector<double>& g)
{
  if (g.empty())
    return;
  gain = g;
}

//--------------------------------------------------------------------------
// Process some data
void AttenuatorFilter::accept(FragmentPtr fragment)
{
  auto gain_used = size_t{};
  for (auto& wit: fragment->waveforms)
  {
    auto& w = wit.second;
    auto g = gain.begin();
    for (auto s = 0u; s < w.size(); ++s)
    {
      w[s] *= *g;
      if (g != gain.end() - 1)
        ++g;
    }
    if (w.size() > gain_used)
      gain_used = w.size();
  }
  gain_used = min(gain_used, gain.size() - 1);
  gain.erase(gain.begin(), gain.begin() + gain_used);

  send(fragment);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "attenuator",
  "Audio attenuator",
  "Audio attenuator / gain control",
  "audio",
  {
    { "gain", { "Gain level (0-1)", Value::Type::number,
                { &AttenuatorFilter::get_gain, &AttenuatorFilter::set_gain },
                true } },
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AttenuatorFilter, module)

