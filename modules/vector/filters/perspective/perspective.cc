//==========================================================================
// ViGraph dataflow module: vector/filters/perspective/perspective.cc
//
// 3D perspective filter
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include <cmath>

namespace {

//==========================================================================
// Perspective filter
class PerspectiveFilter: public FrameFilter
{
  double distance;  // Horizon distance

  // Filter/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FramePtr frame) override;

public:
  // Construct
  PerspectiveFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//  <perspective distance="10"/>
PerspectiveFilter::PerspectiveFilter(const Dataflow::Module *module,
                                     const XML::Element& config):
  FrameFilter(module, config)
{
  distance = config.get_attr_real("d", 1);
}

//--------------------------------------------------------------------------
// Set a control property
void PerspectiveFilter::set_property(const string& property, const SetParams& sp)
{
  if (property == "d") update_prop(distance, sp);
}

//--------------------------------------------------------------------------
// Process some data
void PerspectiveFilter::accept(FramePtr frame)
{
  // Modify all points in the frame
  for(auto& p: frame->points)
  {
    if (distance+p.z > 0)
    {
      double f = distance / (distance+p.z);
      p.x *= f;
      p.y *= f;
    }
    else p.c = Colour::black;  // blank it
  }

  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "perspective",
  "Perspective",
  "Perspective scaling over Z distance",
  "vector",
  {
    { "d", { {"Horizon distance","1"}, Value::Type::number, "d", true } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(PerspectiveFilter, module)
