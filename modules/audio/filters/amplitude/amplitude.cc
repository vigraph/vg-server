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
class AmplitudeFilter: public FragmentFilter, public Dataflow::ControlImpl
{
private:
  double amplitude{0.0};

  // Source/Element virtuals
  void pre_tick(const TickData& td) override;
  void accept(FragmentPtr fragment) override;

  // Add control JSON
  JSON::Value get_json(const string& path) const override
  { JSON::Value json=Element::get_json(path); add_to_json(json); return json; }

public:
  double scale{1.0};
  double offset{0.0};

  AmplitudeFilter(const Dataflow::Module *module):
    FragmentFilter(module), ControlImpl(module) {}
};

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
  ControlImpl::send(amplitude * scale + offset);
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
    { "scale",  { "Scale to apply to amplitude",
                  Value::Type::number, &AmplitudeFilter::scale, true } },
    { "offset", { "Offset to apply to amplitude",
                  Value::Type::number, &AmplitudeFilter::offset, true } }
  },
  { { "output", { "Amplitude", "value", Value::Type::number }}},
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(AmplitudeFilter, module)
