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
    for (const auto& wit: fragment->waveforms)
    {
      const auto c = wit.first;
      const auto& w = wit.second;

      auto cwit = combined->waveforms.find(c);
      if (cwit == combined->waveforms.end())
      {
        combined->waveforms[c] = w;
      }
      else
      {
        auto& cw = cwit->second;
        cw.resize(max(cw.size(), w.size()));
        for (auto i = 0u; i < cw.size(); ++i)
        {
          switch (mode)
          {
            case Mode::add:
              cw[i] += w[i];
              break;
            case Mode::multiply:
              cw[i] *= w[i];
              break;
          }
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
