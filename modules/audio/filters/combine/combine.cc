//==========================================================================
// ViGraph dataflow module:
//    audio/filters/combine/combine.cc
//
// Audio combine filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"

namespace {

using namespace ViGraph::Dataflow;

//==========================================================================
// Combine filter
class CombineFilter: public FragmentFilter
{
  MT::Mutex mutex;
  enum class Mode
  {
    add,
    multiply
  };
  Mode mode = Mode::add;
  shared_ptr<Fragment> combined{nullptr};

  // Source/Element virtuals
  void accept(FragmentPtr fragment) override;
  void pre_tick(const TickData& td) override;
  void post_tick(const TickData& td) override;

public:
  CombineFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <combine/>
CombineFilter::CombineFilter(const Dataflow::Module *module,
                             const XML::Element& config):
    Element(module, config), FragmentFilter(module, config)
{
  const string& m = config["mode"];
  if (m.empty() || m == "add")
    mode = Mode::add;
  else if (m == "multiply")
    mode = Mode::multiply;
  else
  {
    Log::Error log;
    log << "Unknown mode '" << m << "' in Combine '" << id << "'\n";
  }
}

//--------------------------------------------------------------------------
// Process some data
void CombineFilter::accept(FragmentPtr fragment)
{
  MT::Lock lock{mutex};
  // If this is the first, just keep it
  if (!combined)
  {
    // Take it over
    combined = fragment;
  }
  else
  {
    // Make sure fragment with most channels is in combined, because extending
    // channels within an existing fragment is no fun
    if (fragment->nchannels > combined->nchannels)
      combined.swap(fragment);

    // Extend number of samples if new fragment has more
    const auto samples_per_channel = fragment->waveform.size()
                                     / fragment->nchannels;
    if (samples_per_channel > combined->waveform.size() / combined->nchannels)
      combined->waveform.resize(samples_per_channel * combined->nchannels);

    // Add new fragment to existing
    for (auto i = 0u; i < samples_per_channel; ++i)
    {
      for (auto c = 0u; c < fragment->nchannels; ++c)
      {
        switch (mode)
        {
          case Mode::add:
            combined->waveform[i * combined->nchannels + c]
                            += fragment->waveform[i * fragment->nchannels + c];
            break;
          case Mode::multiply:
            combined->waveform[i * combined->nchannels + c]
                            *= fragment->waveform[i * fragment->nchannels + c];
        }
      }
    }
  }
}

//--------------------------------------------------------------------------
// Pre-tick setup
void CombineFilter::pre_tick(const TickData&)
{
  MT::Lock lock{mutex};
  combined.reset();
}

//--------------------------------------------------------------------------
// Post-tick flush
void CombineFilter::post_tick(const TickData&)
{
  MT::Lock lock{mutex};
  if (!!combined) send(combined);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "combine",
  "Audio combine",
  "Audio combine (additive or multiplicative)",
  "audio",
  { },  // no properties
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(CombineFilter, module)
