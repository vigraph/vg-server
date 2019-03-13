//==========================================================================
// ViGraph dataflow module: vector/filters/rotate/rotate.cc
//
// 3D rotation filter
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include <cmath>

namespace {

//==========================================================================
// Rotation filter
class RotateFilter: public FrameFilter
{
  double rx{0.0};  // 0..1 - 1 = 360 deg = 2pi rad
  double ry{0.0};
  double rz{0.0};

  // Filter/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void accept(FramePtr frame) override;

public:
  // Construct
  RotateFilter(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//  <rotate x="0.25" y="0" z="0"/>
RotateFilter::RotateFilter(const Dataflow::Module *module,
                           const XML::Element& config):
  FrameFilter(module, config)
{
  rx = config.get_attr_real("x");
  ry = config.get_attr_real("y");
  rz = config.get_attr_real("z");
}

//--------------------------------------------------------------------------
// Set a control property
void RotateFilter::set_property(const string& property, const SetParams& sp)
{
       if (property == "x") update_prop(rx, sp);
  else if (property == "y") update_prop(ry, sp);
  else if (property == "z") update_prop(rz, sp);
}

//--------------------------------------------------------------------------
// Process some data
void RotateFilter::accept(FramePtr frame)
{
  // Precalculate useful stuff
  double sinx = sin(rx*2*pi);
  double cosx = cos(rx*2*pi);
  double siny = sin(ry*2*pi);
  double cosy = cos(ry*2*pi);
  double sinz = sin(rz*2*pi);
  double cosz = cos(rz*2*pi);

  // Modify all points in the frame
  for(auto& p: frame->points)
  {
    double xy = cosx*p.y - sinx*p.z;
    double xz = sinx*p.y + cosx*p.z;
    double yz = cosy*xz  - siny*p.x;
    double yx = siny*xz  + cosy*p.x;
    double zx = cosz*yx  - sinz*xy;
    double zy = sinz*yx  + cosz*xy;

    p.x = zx;
    p.y = zy;
    p.z = yz;
  }

  send(frame);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "rotate",
  "Rotate",
  "3D rotation",
  "vector",
  {
    { "x", { "Angle to rotate on X axis", Value::Type::number, "x", true } },
    { "y", { "Angle to rotate on Y axis", Value::Type::number, "y", true } },
    { "z", { "Angle to rotate on Z axis", Value::Type::number, "z", true } }
  },
  { "VectorFrame" }, // inputs
  { "VectorFrame" }  // outputs
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(RotateFilter, module)
