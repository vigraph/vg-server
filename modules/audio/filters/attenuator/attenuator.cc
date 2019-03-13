//==========================================================================
// ViGraph dataflow module:
//    audio/filters/attenuator/attenuator.cc
//
// Audio attenuator filter
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"
#include <algorithm>

namespace {

using namespace ViGraph::Dataflow;

//==========================================================================
// Attenuator filter
class AttenuatorFilter: public FragmentFilter
{
  double last_gain{0.0};
  double gain{1.0};
  bool interpolate = true;

  // Source/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FragmentPtr fragment) override;

public:
  AttenuatorFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <attenuator gain="0.5"/>
AttenuatorFilter::AttenuatorFilter(const Dataflow::Module *module,
                                   const XML::Element& config):
    FragmentFilter(module, config)
{
  gain = config.get_attr_real("gain", 1.0);
  interpolate = config.get_attr_bool("interpolate", interpolate);
}

//--------------------------------------------------------------------------
// Set a control property
void AttenuatorFilter::set_property(const string& property, const SetParams& sp)
{
  if (property == "gain")
    update_prop(gain, sp);
  else if (property == "interpolate")
    update_prop(interpolate, sp);
}

//--------------------------------------------------------------------------
// Process some data
void AttenuatorFilter::accept(FragmentPtr fragment)
{
  for (auto& wit: fragment->waveforms)
  {
    auto& w = wit.second;
    for (auto s = 0u; s < w.size(); ++s)
    {
      if (interpolate)
        w[s] *= last_gain + ((gain - last_gain) * s / w.size());
      else
        w[s] *= gain;
    }
  }

  last_gain = gain;

  send(fragment);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "attenuator",
  "Audio attenuator",
  "Audio attenuator / gain control",
  "audio",
  {
    { "gain", { {"Gain level (0-1)", "1"}, Value::Type::number,
                                             "@gain", true } },
    { "interpolate", { {"Interpolate up from last gain", "1"},
                       Value::Type::boolean, "@interpolate", true } }
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AttenuatorFilter, module)

