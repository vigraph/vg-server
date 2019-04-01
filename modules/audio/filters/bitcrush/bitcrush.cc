//==========================================================================
// ViGraph dataflow module:
//    audio/filters/bitcrush/bitcrush.cc
//
// Audio bitcrush filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"

namespace {

using namespace ViGraph::Dataflow;

//==========================================================================
// BitCrush filter
class BitCrushFilter: public FragmentFilter
{
  struct State
  {
    unsigned samples_seen = 0;
    sample_t last_sample = 0.0;
  };
  map<Speaker, State> state;
  bool enabled = true;

  // Source/Element virtuals
  void update() override;
  void accept(FragmentPtr fragment) override;
  void notify_target_of(Element *, const string& property) override;

public:
  int rate = 1;
  int bits = 32;

  using FragmentFilter::FragmentFilter;

  void on() { enabled = true; }
  void off() { enabled = false; }
};

//--------------------------------------------------------------------------
// Update
void BitCrushFilter::update()
{
  rate = max(rate, 1);
  bits = min(max(bits, 1), 32);
}

//--------------------------------------------------------------------------
// Process some data
void BitCrushFilter::accept(FragmentPtr fragment)
{
  if (enabled)
  {
    const auto max = pow(2, bits) - 1;
    for (auto& wit: fragment->waveforms)
    {
      const auto c = wit.first;
      auto& w = wit.second;

      auto& st = state[c];
      for (auto i = 0u; i < w.size(); ++i)
      {
        if (st.samples_seen++ % rate)
        {
          w[i] = st.last_sample;
        }
        else
        {
          w[i] = (2.0 * (round(((w[i] + 1.0) * 0.5) * max) / max)) - 1.0;
          st.last_sample = w[i];
        }
      }
    }
  }

  send(fragment);
}

//--------------------------------------------------------------------------
// If recipient of triggers default to disabled
void BitCrushFilter::notify_target_of(Element *, const string& property)
{
  if (property == "enable")
    enabled = false;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "bitcrush",
  "Audio bitcrush",
  "Audio bitcrush",
  "audio",
  {
    { "rate", { "Rate reduction", Value::Type::number,
                &BitCrushFilter::rate, true } },
    { "bits", { "Bit depth", Value::Type::number,
                &BitCrushFilter::bits, true } },
    { "enable", { "Enable the filter", Value::Type::trigger,
                  &BitCrushFilter::on, true } },
    { "disable", { "Disable the filter", Value::Type::trigger,
                    &BitCrushFilter::off, true } },
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(BitCrushFilter, module)
