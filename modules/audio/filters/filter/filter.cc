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
private:
  enum class Mode
  {
    low_pass,
    high_pass,
    band_pass
  };
  Mode mode = Mode::low_pass;

  double feedback = 0.0;
  unsigned steepness = 1;
  bool enabled = true;

  map<Speaker, vector<sample_t>> buffers; // Vector of buffers per speaker

  // Source/Element virtuals
  void update() override;
  void accept(FragmentPtr fragment) override;
  void notify_target_of(const string& property) override;

public:
  double cutoff = 1.0;
  double resonance = 1.0;

  using FragmentFilter::FragmentFilter;

  // Getters/Setters
  string get_mode() const;
  void set_mode(const string& mode);
  int get_steepness() const { return steepness; }
  void set_steepness(int steepness);
  void on() { enabled = true; }
  void off() { enabled = false; buffers.clear(); }
};

//--------------------------------------------------------------------------
// Update
void FilterFilter::update()
{
  cutoff = min(max(cutoff, 0.0), 1.0);
  resonance = min(max(resonance, 0.0), 1.0);
  feedback = resonance + resonance / (1.0 - min(max(cutoff, 0.0), 0.9999));
}

//--------------------------------------------------------------------------
// Get mode
string FilterFilter::get_mode() const
{
  switch (mode)
  {
    case Mode::low_pass: return "low-pass";
    case Mode::high_pass: return "high-pass";
    case Mode::band_pass: return "band-pass";
  }
}

//--------------------------------------------------------------------------
// Set mode
void FilterFilter::set_mode(const string& m)
{
  if (m == "low-pass")
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
}

//--------------------------------------------------------------------------
// Get steepness
void FilterFilter::set_steepness(int s)
{
  steepness = max(s, 1);
  for (auto& bit: buffers)
    bit.second.resize(steepness + 1);
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
void FilterFilter::notify_target_of(const string& property)
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
    { "mode", { "Filter mode", Value::Type::choice,
                { &FilterFilter::get_mode, &FilterFilter::set_mode },
                { "low-pass", "high-pass", "band-pass" }, true } },
    { "cutoff", { "Cutoff level (0-1)", Value::Type::number,
                  &FilterFilter::cutoff, true } },
    { "resonance", { "Resonance level (0-1)", Value::Type::number,
                     &FilterFilter::resonance, true } },
    { "steepness", { "Steepness level (1-?)", Value::Type::number,
                     { &FilterFilter::get_steepness,
                       &FilterFilter::set_steepness }, true } },
    { "enable", { "Enable the filter", Value::Type::trigger,
                  &FilterFilter::on, true } },
    { "disable", { "Disable the filter", Value::Type::trigger,
                   &FilterFilter::off, true } },
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(FilterFilter, module)
