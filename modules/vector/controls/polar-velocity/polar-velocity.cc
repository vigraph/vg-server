//==========================================================================
// ViGraph dataflow module: vector/controls/polar-velocity/polar-velocity.cc
//
// Animation control to apply x,y movement with a velocity at an angle
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include <cmath>

namespace {

//==========================================================================
// PolarVelocity control
class PolarVelocityControl: public Dataflow::Control
{
  Vector p, start_p;    // Current and start position
  double angle{0.0};    // 0..1 full circle, positive anticlockwise, 0 = +x
  double velocity{0.0}; // Distance per second
  Dataflow::timestamp_t last_tick{-1.0};

  // Control/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void pre_tick(const TickData& td) override;
  void enable() override;

public:
  // Construct
  PolarVelocityControl(const Dataflow::Module *module,
                       const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
// <polar-velocity x="1" y="1"
//                 v="0.1" angle="0.25"
//                 property-x="x" property-y="y"/>
PolarVelocityControl::PolarVelocityControl(const Dataflow::Module *module,
                                           const XML::Element& config):
  Control(module, config)
{
  start_p.x = config.get_attr_real("x");
  start_p.y = config.get_attr_real("y");
  p = start_p;
  angle = config.get_attr_real("angle");
  velocity = config.get_attr_real("v");
}

//--------------------------------------------------------------------------
// Set a control property
void PolarVelocityControl::set_property(const string& property,
                                        const SetParams& sp)
{
       if (property == "angle") update_prop(angle, sp);
  else if (property == "v")     update_prop(velocity, sp);
  // !!! Need setters to replicate this rest of start_p and p
  else if (property == "x") { update_prop(start_p.x, sp); p.x=start_p.x; }
  else if (property == "y") { update_prop(start_p.y, sp); p.y=start_p.y; }
}

//--------------------------------------------------------------------------
// Enable (reset)
void PolarVelocityControl::enable()
{
  last_tick = -1.0;
  p = start_p;
}

//--------------------------------------------------------------------------
// Tick
void PolarVelocityControl::pre_tick(const TickData& td)
{
  if (last_tick >= 0.0)
  {
    auto delta_t = td.t - last_tick;
    auto distance = velocity * delta_t;
    auto rad = angle * 2 * pi;
    Vector delta(distance * cos(rad), distance * sin(rad));
    p += delta;
    send("x", Dataflow::Value{p.x});
    send("y", Dataflow::Value{p.y});
  }

  last_tick = td.t;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "polar-velocity",
  "Polar Velocity",
  "Apply velocity along a rotating vector to a Translate",
  "vector",
  {
    { "x", { "Current X co-ordinate", Value::Type::number, true } },
    { "y", { "Current Y co-ordinate", Value::Type::number, true } },
    { "z", { "Current Z co-ordinate", Value::Type::number, true } },
    { "angle", { "Angle of acceleration (0..1)", Value::Type::number, true } },
    { "v",     { "Velocity to apply", Value::Type::number, true } }
  },
  {
    { "x", { "X component of velocity", "x", Value::Type::number }},
    { "y", { "Y component of velocity", "y", Value::Type::number }}
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(PolarVelocityControl, module)
