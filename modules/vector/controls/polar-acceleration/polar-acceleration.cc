//==========================================================================
// ViGraph dataflow module:
//  vector/controls/polar-acceleration/polar-acceleration.cc
//
// Animation control to apply acceleration at an angle on a 2D x,y plane
// attached to a <velocity>
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include <cmath>

namespace {

//==========================================================================
// PolarAcceleration control
class PolarAccelerationControl: public Dataflow::Control
{
  double angle{0.0};         // 0..1 full circle, positive anticlockwise, 0 = +x
  double acceleration{0.0};  // Distance per second per second
  Dataflow::timestamp_t last_tick{-1.0};

  // Control/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void pre_tick(const TickData& td) override;

public:
  // Construct
  PolarAccelerationControl(const Dataflow::Module *module,
                           const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
// <polar-acceleration a="0.1" angle="0.25"
//                 property-x="x" property-y="y"/>
PolarAccelerationControl::PolarAccelerationControl(
                                      const Dataflow::Module *module,
                                      const XML::Element& config):
  Element(module, config), Control(module, config)
{
  angle = config.get_attr_real("angle");
  acceleration = config.get_attr_real("a");
}

//--------------------------------------------------------------------------
// Set a control property
void PolarAccelerationControl::set_property(const string& property,
                                        const SetParams& sp)
{
       if (property == "angle") update_prop(angle, sp);
  else if (property == "a")     update_prop(acceleration, sp);
}

//--------------------------------------------------------------------------
// Tick
void PolarAccelerationControl::pre_tick(const TickData& td)
{
  if (last_tick >= 0.0)
  {
    auto delta_t = td.t - last_tick;
    auto delta_v = acceleration * delta_t;
    auto rad = angle * 2 * pi;
    auto cos_rad = cos(rad);
    auto sin_rad = sin(rad);
    Vector delta(delta_v * cos_rad, delta_v * sin_rad);

    SetParams setx(Dataflow::Value{delta.x}, true);
    send("x", setx);
    SetParams sety(Dataflow::Value{delta.y}, true);
    send("y", sety);
  }

  last_tick = td.t;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "polar-acceleration",
  "Polar Acceleration",
  "Apply acceleration along a rotating vector to a Velocity",
  "vector",
  {
    { "angle", { "Angle of acceleration (0..1)", Value::Type::number, true } },
    { "a",     { "Acceleration to apply", Value::Type::number, true } }
  },
  {
    { "x", { "X component of acceleration", "x", Value::Type::number }},
    { "y", { "Y component of acceleration", "y", Value::Type::number }}
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(PolarAccelerationControl, module)

