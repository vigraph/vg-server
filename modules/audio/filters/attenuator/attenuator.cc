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
public:
  double gain{1.0};
  bool interpolate = true;

private:
  double last_gain{0.0};

  // Source/Element virtuals
  void accept(FragmentPtr fragment) override;

public:
  using FragmentFilter::FragmentFilter;
};

//--------------------------------------------------------------------------
// Process some data
void AttenuatorFilter::accept(FragmentPtr fragment)
{
  for (auto& wit: fragment->waveforms)
  {
    auto& w = wit.second;
    for (auto s = 0u; s < w.size(); ++s)
    {
      if (interpolate)
        w[s] *= last_gain + ((gain - last_gain) * s / w.size());
      else
        w[s] *= gain;
    }
  }

  last_gain = gain;

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
                &AttenuatorFilter::gain, true } },
    { "interpolate", { "Interpolate up from last gain", Value::Type::boolean,
                       &AttenuatorFilter::interpolate, true } }
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AttenuatorFilter, module)

