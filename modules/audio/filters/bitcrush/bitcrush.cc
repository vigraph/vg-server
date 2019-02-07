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

  unsigned samples_seen = 0;
  sample_t last_sample = 0.0;

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
  for (auto i = 0u; i < fragment->waveform.size() / fragment->nchannels; ++i)
  {
    for (auto c = 0u; c < fragment->nchannels; ++c)
    {
      auto& s = fragment->waveform[i * fragment->nchannels + c];
      if (samples_seen++ % rate)
      {
        s = last_sample;
      }
      else
      {
        const auto max = pow(2, bits) - 1;
        s = (2.0 * (round(((s + 1.0) * 0.5) * max) / max)) - 1.0;
        last_sample = s;
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
