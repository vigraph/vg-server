//==========================================================================
// ViGraph dataflow module:
//    audio/filters/vu-meter/vu-meter.cc
//
// Audio VU meter filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"
#include "vg-geometry.h"

namespace {

using namespace ViGraph::Dataflow;

//==========================================================================
// VU meter filter
class VUMeterFilter: public FragmentFilter, public Dataflow::Control
{
  double level = 0.0;
  const double charge = 0.1;
  const double discharge = 0.001;

  // Source/Element virtuals
  void accept(FragmentPtr fragment) override;
  void post_tick(const TickData&) override;

public:
  VUMeterFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <vu-meter/>
VUMeterFilter::VUMeterFilter(const Dataflow::Module *module,
                             const XML::Element& config):
    Element(module, config), FragmentFilter(module, config),
    Control(module, config, true)  // optional targets
{
}

//--------------------------------------------------------------------------
// Process some data
void VUMeterFilter::accept(FragmentPtr fragment)
{
  for (auto i = 0u; i < fragment->waveform.size() / fragment->nchannels; ++i)
  {
    auto s = 0.0l;
    for (auto c = 0; c < fragment->nchannels; ++c)
    {
      s += fragment->waveform[i * fragment->nchannels + c];
    }
    s /= fragment->nchannels;

    s = abs(s);
    if (s > level)
      level = level * (1 - charge) + s * charge;
    else
      level = level * (1 - discharge);
  }
  Generator::send(fragment);
}

//--------------------------------------------------------------------------
// Send control message
void VUMeterFilter::post_tick(const TickData&)
{
  Control::send(SetParams{Dataflow::Value{level}});
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "vu-meter",
  "Audio VU meter",
  "Audio VU meter",
  "audio",
  { }, // no properties
  { { "", { "VU output", "", Value::Type::number }}},
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(VUMeterFilter, module)
