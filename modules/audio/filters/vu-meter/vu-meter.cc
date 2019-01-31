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
  const auto rise_fall_time = 0.3l;   // VU meter standard rise time is 300ms
  const auto dt = 1.0l / sample_rate; // Sample interval
  const auto a = dt / (rise_fall_time + dt);  // Low pass alpha
  for (auto i = 0u; i < fragment->waveform.size() / fragment->nchannels; ++i)
  {
    auto s = 0.0l;
    for (auto c = 0; c < fragment->nchannels; ++c)
    {
      s += fragment->waveform[i * fragment->nchannels + c];
    }
    s /= fragment->nchannels;
    s = abs(s);

    // Low pass filter
    level = a * s + (1 - a) * level;
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
