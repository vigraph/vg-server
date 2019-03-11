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
  double resonance = 1.0;
  double feedback = 0.0;
  unsigned steepness = 1;
  bool enabled = true;

  map<Speaker, vector<sample_t>> buffers; // Vector of buffers per speaker

  void update_feedback()
  {
    feedback = resonance + resonance / (1.0 - min(max(cutoff, 0.0), 0.9999));
  }

  // Source/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FragmentPtr fragment) override;
  void notify_target_of(Element *, const string& property) override;

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
  else if (m == "high-pass")
    mode = Mode::high_pass;
  else if (m == "band-pass")
    mode = Mode::band_pass;
  else
  {
    Log::Error log;
    log << "Unknown mode '" << m << "' in Filter '" << id << "'\n";
  }

  cutoff = min(max(config.get_attr_real("cutoff", 1.0), 0.0), 1.0);
  resonance = min(max(config.get_attr_real("cutoff", 1.0), 0.0), 1.0);
  steepness = max(config.get_attr_int("steepness", steepness), 1);
}

//--------------------------------------------------------------------------
// Set a control property
void FilterFilter::set_property(const string& property, const SetParams& sp)
{
  if (property == "cutoff")
    update_prop(cutoff, sp);
  else if (property == "steepness")
  {
    steepness = max(sp.v.d, 1.0);
    for (auto& bit: buffers)
    {
      bit.second.resize(steepness + 1);
    }
  }
  else if (property == "resonance")
    update_prop(resonance, sp);
  else if (property == "enable")
    enabled = true;
  else if (property == "disable")
  {
    enabled = false;
    buffers.clear();
  }
  update_feedback();
}

//--------------------------------------------------------------------------
// Process some data
void FilterFilter::accept(FragmentPtr fragment)
{
  if (enabled)
  {
    for (auto& wit: fragment->waveforms)
    {
      const auto c = wit.first;
      auto& w = wit.second;

      auto bit = buffers.find(c);
      if (bit == buffers.end())
      {
        bit = buffers.emplace(c, vector<sample_t>(steepness + 1)).first;
      }
      auto& bf = bit->second;

      for (auto i = 0u; i < w.size(); ++i)
      {
        for (auto b = 0u; b < bf.size(); ++b)
        {
          if (!b)
            bf[b] += cutoff * (w[i] - bf[b] + feedback * (bf[b] - bf[b + 1]));
          else
            bf[b] += cutoff * (bf[b - 1] - bf[b]);
        }
        switch (mode)
        {
          case Mode::low_pass:
            w[i] = bf.back();
            break;
          case Mode::high_pass:
            w[i] = w[i] - bf.back();
            break;
          case Mode::band_pass:
            w[i] = bf.front() - bf.back();
            break;
        }
      }
    }
  }

  send(fragment);
}

//--------------------------------------------------------------------------
// If recipient of triggers default to disabled
void FilterFilter::notify_target_of(Element *, const string& property)
{
  if (property == "enable")
    enabled = false;
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
    { "resonance", { {"Resonance level (0-1)", "1"}, Value::Type::number,
                                                     "@resonance", true } },
    { "steepness", { {"Steepness level (1-?)", "1"}, Value::Type::number,
                                                     "@steepness", true } },
    { "enable", { "Enable the filter", Value::Type::trigger, true } },
    { "disable", { "Disable the filter", Value::Type::trigger, true } },
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(FilterFilter, module)
