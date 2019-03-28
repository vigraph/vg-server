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
public:
  double angle{0.0};    // 0..1 full circle, positive anticlockwise, 0 = +x
  double velocity{0.0}; // Distance per second

private:
  Vector p, start_p;    // Current and start position
  Dataflow::timestamp_t last_tick{-1.0};

  // Control/Element virtuals
  void pre_tick(const TickData& td) override;
  void enable() override;

public:
  using Control::Control;

  // Getters/Setters
  double get_x() const { return p.x; }
  void set_x(double x) { p.x = start_p.x = x; }
  double get_y() const { return p.y; }
  void set_y(double y) { p.y = start_p.y = y; }
};

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
    { "x", { "Current X co-ordinate", Value::Type::number,
             { &PolarVelocityControl::get_x, &PolarVelocityControl::set_x },
             true } },
    { "y", { "Current Y co-ordinate", Value::Type::number,
             { &PolarVelocityControl::get_y, &PolarVelocityControl::set_y },
             true } },
    { "angle", { "Angle of acceleration (0..1)", Value::Type::number,
                 &PolarVelocityControl::angle, true } },
    { "v",     { "Velocity to apply", Value::Type::number,
                 &PolarVelocityControl::velocity, true } }
  },
  {
    { "x", { "X component of velocity", "x", Value::Type::number }},
    { "y", { "Y component of velocity", "y", Value::Type::number }}
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(PolarVelocityControl, module)
