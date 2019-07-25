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
  using FragmentFilter::FragmentFilter;

  // Getters/Setters
  string get_mode() const;
  void set_mode(const string& mode);
};

//--------------------------------------------------------------------------
// Get mode
string CombineFilter::get_mode() const
{
  switch (mode)
  {
    case Mode::add: return "add";
    case Mode::multiply: return "multiply";
  }
  return "";
}

//--------------------------------------------------------------------------
// Set mode
void CombineFilter::set_mode(const string& m)
{
  if (m == "add")
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
        const auto cl = cw.size();
        if (w.size() > cl)
        {
          cw.resize(w.size());
          copy(&w[cl], &w[w.size() - cl], &cw[cl]);
        }
        for (auto i = 0u; i < min(cl, w.size()); ++i)
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
  combined.reset();
}

//--------------------------------------------------------------------------
// Post-tick flush
void CombineFilter::post_tick(const TickData&)
{
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
  {
    { "mode", { "How the inputs are combined", Value::Type::choice,
                { &CombineFilter::get_mode, &CombineFilter::set_mode },
                { "add", "multiply" } } }
  },  // no properties
  { { "Audio", true } }, // multiple inputs
  { "Audio" }            // single output
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(CombineFilter, module)
