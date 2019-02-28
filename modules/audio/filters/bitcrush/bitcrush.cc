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
  unsigned rate = 1;
  unsigned bits = 32;

  struct State
  {
    unsigned samples_seen = 0;
    sample_t last_sample = 0.0;
  };
  map<Speaker, State> state;

  // Source/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FragmentPtr fragment) override;

public:
  BitCrushFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <bitcrush time="0.125" />
BitCrushFilter::BitCrushFilter(const Dataflow::Module *module,
                         const XML::Element& config):
    Element(module, config), FragmentFilter(module, config)
{
  rate = max(config.get_attr_int("rate", 1), 1);
  bits = min(max(config.get_attr_int("bits", 32), 1), 32);
}

//--------------------------------------------------------------------------
// Set a control property
void BitCrushFilter::set_property(const string& property, const SetParams& sp)
{
  if (property == "rate")
    rate = max(static_cast<int>(sp.v.d), 1);
  else if (property == "bits")
    bits = min(max(static_cast<int>(sp.v.d), 1), 32);
}

//--------------------------------------------------------------------------
// Process some data
void BitCrushFilter::accept(FragmentPtr fragment)
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

  send(fragment);
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
    { "rate", { {"Rate reduction", "1-?"}, Value::Type::number,
                                           "@rate", true } },
    { "bits", { {"Bit depth", "1-32"}, Value::Type::number,
                                       "@bits", true } },
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(BitCrushFilter, module)
