//==========================================================================
// ViGraph dataflow module:
//    audio/filters/amplitude/amplitude.cc
//
// Audio amplitude filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"
#include "vg-geometry.h"

namespace {

using namespace ViGraph::Dataflow;

//==========================================================================
// VU meter filter
class AmplitudeFilter: public FragmentFilter, public Dataflow::Control
{
  double scale{1.0};
  double offset{0.0};
  double amplitude{0.0};

  // Control virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void pre_tick(const TickData& td) override;

  // Source/Element virtuals
  void accept(FragmentPtr fragment) override;

public:
  AmplitudeFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <amplitude/>
AmplitudeFilter::AmplitudeFilter(const Dataflow::Module *module,
                             const XML::Element& config):
    Element(module, config), FragmentFilter(module, config),
    Control(module, config)
{
  scale = config.get_attr_real("scale", 1.0);
  offset = config.get_attr_real("offset");
}

//--------------------------------------------------------------------------
// Set a control property
void AmplitudeFilter::set_property(const string& property,
                                   const SetParams& sp)
{
  if (property == "scale")
    update_prop(scale, sp);
  else if (property == "offset")
    update_prop(offset, sp);
}

//--------------------------------------------------------------------------
// Process some data
void AmplitudeFilter::accept(FragmentPtr fragment)
{
  auto ssum = 0.0l;
  auto samples = 0ul;
  for (const auto& wit: fragment->waveforms)
  {
    for (const auto& s: wit.second)
      ssum += s * s;
    samples += wit.second.size();
  }
  amplitude = sqrt(ssum / samples);
  Generator::send(fragment);
}

//--------------------------------------------------------------------------
// Send latest reading
void AmplitudeFilter::pre_tick(const TickData&)
{
  Control::send(SetParams{Dataflow::Value{amplitude * scale + offset}});
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "amplitude",
  "Audio amplitude",
  "Audio amplitude",
  "audio",
  {
    { "scale",  { {"Scale to apply to amplitude", "1.0"},
          Value::Type::number, "@scale", true } },
    { "offset", { {"Offset to apply to amplitude", "0"},
          Value::Type::number, "@offset", true } }
  },
  { { "", { "Amplitude", "", Value::Type::number }}},
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AmplitudeFilter, module)
