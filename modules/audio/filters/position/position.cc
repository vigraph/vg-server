//==========================================================================
// ViGraph dataflow module:
//    audio/filters/position/position.cc
//
// Audio position filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module.h"
#include "vg-geometry.h"

namespace {

using namespace ViGraph::Dataflow;
using namespace ViGraph::Geometry;

//==========================================================================
// Position filter
class PositionFilter: public FragmentFilter
{
  Vector start_d, d;

  // Source/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FragmentPtr fragment) override;
  void enable() override;

public:
  PositionFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML:
//   <position x="0.5" y="1.0" z="0.33"/>
PositionFilter::PositionFilter(const Dataflow::Module *module,
                               const XML::Element& config):
    FragmentFilter(module, config)
{
  d.x = config.get_attr_real("x");
  d.y = config.get_attr_real("y");
  d.z = config.get_attr_real("z");
  start_d = d;
}

//--------------------------------------------------------------------------
// Set a control property
void PositionFilter::set_property(const string& property, const SetParams& sp)
{
       if (property == "x") update_prop(d.x, sp);
  else if (property == "y") update_prop(d.y, sp);
  else if (property == "z") update_prop(d.z, sp);
}

//--------------------------------------------------------------------------
// Enable (reset)
void PositionFilter::enable()
{
  d = start_d;
}

//--------------------------------------------------------------------------
// Process some data
void PositionFilter::accept(FragmentPtr f)
{
  const auto wit = f->waveforms.find(Speaker::front_center);
  if (wit == f->waveforms.end())
    return;

  auto fragment = new Fragment(f->timestamp);
  const auto& wc = wit->second;
  auto& wl = fragment->waveforms[Speaker::front_left];
  auto& wr = fragment->waveforms[Speaker::front_right];
  wl.resize(wc.size());
  wr.resize(wc.size());

  auto pos = d.x;
  if (pos > 0.5) pos = 0.5;
  if (pos < -0.5) pos = -0.5;

  for (auto i = 0u; i < wc.size(); ++i)
  {
    wl[i] = wc[i] * cos((pos + 0.5) * pi / 2);
    wr[i] = wc[i] * sin((pos + 0.5) * pi / 2);
  }

  send(fragment);
}


//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "position",
  "Audio position",
  "Audio position",
  "audio",
  {
    { "x", { "Distance to move along X axis", Value::Type::number, "x", true } },
    { "y", { "Distance to move along Y axis", Value::Type::number, "y", true } },
    { "z", { "Distance to move along Z axis", Value::Type::number, "z", true } }
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(PositionFilter, module)
