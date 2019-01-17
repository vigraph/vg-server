//==========================================================================
// ViGraph dataflow module:
//    audio/filters/mixer/mixer.cc
//
// Audio mixer filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"

namespace {

using namespace ViGraph::Dataflow;

//==========================================================================
// Mixer filter
class MixerFilter: public FragmentFilter
{
  shared_ptr<Fragment> sum{nullptr};

  // Source/Element virtuals
  void accept(FragmentPtr fragment) override;
  void pre_tick(Dataflow::timestamp_t) override;
  void post_tick(Dataflow::timestamp_t) override;

public:
  MixerFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <mixer/>
MixerFilter::MixerFilter(const Dataflow::Module *module,
                                   const XML::Element& config):
    Element(module, config), FragmentFilter(module, config)
{
}

//--------------------------------------------------------------------------
// Process some data
void MixerFilter::accept(FragmentPtr fragment)
{
  // If this is the first, just keep it
  if (!sum)
  {
    // Take it over
    sum = fragment;
  }
  else
  {
    // Add to existing, up to matching size
    auto n = min(sum->waveform.size(), fragment->waveform.size());
    for(auto i=0u; i<n; i++)
      sum->waveform[i] += fragment->waveform[i];
  }
}

//--------------------------------------------------------------------------
// Pre-tick setup
void MixerFilter::pre_tick(Dataflow::timestamp_t)
{
  sum.reset();
}

//--------------------------------------------------------------------------
// Post-tick flush
void MixerFilter::post_tick(Dataflow::timestamp_t)
{
  if (!!sum) send(sum);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "mixer",
  "Audio mixer",
  "Audio mixer (additive)",
  "audio",
  { },  // no properties
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(MixerFilter, module)

