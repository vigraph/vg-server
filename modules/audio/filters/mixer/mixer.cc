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
  void pre_tick(const TickData& td) override;
  void post_tick(const TickData& td) override;

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
    // Make sure fragment with most channels is in sum, because extending
    // channels within an existing fragment is no fun
    if (fragment->nchannels > sum->nchannels)
      sum.swap(fragment);

    // Extend number of samples if new fragment has more
    const auto samples_per_channel = fragment->waveform.size()
                                     / fragment->nchannels;
    if (samples_per_channel > sum->waveform.size() / sum->nchannels)
      sum->waveform.resize(samples_per_channel * sum->nchannels);

    // Add new fragment to existing
    for (auto i = 0u; i < samples_per_channel; ++i)
      for (auto c = 0; c < fragment->nchannels; ++c)
        sum->waveform[i * sum->nchannels + c]
                        += fragment->waveform[i * fragment->nchannels + c];
  }
}

//--------------------------------------------------------------------------
// Pre-tick setup
void MixerFilter::pre_tick(const TickData&)
{
  sum.reset();
}

//--------------------------------------------------------------------------
// Post-tick flush
void MixerFilter::post_tick(const TickData&)
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

