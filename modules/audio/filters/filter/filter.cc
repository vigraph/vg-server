//==========================================================================
// ViGraph dataflow module:
//    audio/filters/filter/filter.cc
//
// Audio filter filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"

namespace {

using namespace ViGraph::Dataflow;

//==========================================================================
// Filter filter
class FilterFilter: public FragmentFilter
{
  enum class Mode
  {
    low_pass,
    high_pass,
    band_pass
  };
  Mode mode = Mode::low_pass;

  double cutoff = 1.0;
  unsigned nchannels = 0;
  vector<vector<double>> buff; // Vector of buffers, then vector of channel

  // Source/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FragmentPtr fragment) override;

public:
  FilterFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <filter mode="low-pass"/>
FilterFilter::FilterFilter(const Dataflow::Module *module,
                             const XML::Element& config):
    Element(module, config), FragmentFilter(module, config)
{
  const string& m = config["mode"];
  if (m.empty() || m == "low-pass")
    mode = Mode::low_pass;
  else if (m == "high_pass")
    mode = Mode::high_pass;
  else if (m == "band_pass")
    mode = Mode::band_pass;
  else
  {
    Log::Error log;
    log << "Unknown mode '" << m << "' in Filter '" << id << "'\n";
  }

  cutoff = config.get_attr_real("cutoff", 1.0);

  buff.resize(max(config.get_attr_int("steepness", 1), 1) + 1);
}

//--------------------------------------------------------------------------
// Set a control property
void FilterFilter::set_property(const string& property, const SetParams& sp)
{
  if (property == "cutoff")
    update_prop(cutoff, sp);
  else if (property == "steepness")
    buff.resize(max(sp.v.d, 1.0) + 1);
}

//--------------------------------------------------------------------------
// Process some data
void FilterFilter::accept(FragmentPtr fragment)
{
  if (fragment->nchannels > nchannels)
  {
    nchannels = fragment->nchannels;
    for (auto& b: buff)
    {
      b.resize(nchannels);
    }
  }

  for (auto i = 0u; i < fragment->waveform.size() / fragment->nchannels; ++i)
  {
    for (auto c = 0u; c < fragment->nchannels; ++c)
    {
      auto& s = fragment->waveform[i * fragment->nchannels + c];
      for (auto b = 0u; b < buff.size(); ++b)
      {
        buff[b][c] += cutoff * ((b ? buff[b - 1][c] : s) - buff[b][c]);
      }
      switch (mode)
      {
        case Mode::low_pass:
          s = buff.back()[c];
          break;
        case Mode::high_pass:
          s = s - buff.back()[c];
          break;
        case Mode::band_pass:
          s = buff.front()[c] - buff.back()[c];
          break;
      }
    }
  }

  send(fragment);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "filter",
  "Audio filter",
  "Audio filter (low-, high- or band-pass)",
  "audio",
  {
    { "cutoff", { {"Cutoff level (0-1)", "1"}, Value::Type::number,
                                               "@cutoff", true } },
    { "steepness", { {"Steepness level (1-?)", "1"}, Value::Type::number,
                                                     "@steepness", true } }
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(FilterFilter, module)
