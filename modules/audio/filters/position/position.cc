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
  void setup() override { start_d = d; }
  void accept(FragmentPtr fragment) override;
  void enable() override;

public:
  using FragmentFilter::FragmentFilter;

  // Getters/Setters
  double get_x() const { return d.x; }
  void set_x(double x) { d.x = x; }
  double get_y() const { return d.y; }
  void set_y(double y) { d.y = y; }
  double get_z() const { return d.z; }
  void set_z(double z) { d.z = z; }
};

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
    { "x", { "Distance to move along X axis", Value::Type::number,
             { &PositionFilter::get_x, &PositionFilter::set_x }, true } },
    { "y", { "Distance to move along Y axis", Value::Type::number,
             { &PositionFilter::get_y, &PositionFilter::set_y }, true } },
    { "z", { "Distance to move along Z axis", Value::Type::number,
             { &PositionFilter::get_z, &PositionFilter::set_z },
             true } },
  },
  { "Audio" }, // inputs
  { "Audio" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(PositionFilter, module)
