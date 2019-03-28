//==========================================================================
// ViGraph dataflow module: vector/controls/velocity/velocity.cc
//
// Animation control to apply Cartesian x,y,z velocity to a <translate>
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module.h"
#include <cmath>

namespace {

//==========================================================================
// Velocity control
class VelocityControl: public Dataflow::Control
{
public:
  bool wait{false};

private:
  Vector v;   // Velocity
  Vector p;   // Position
  Vector start_p;
  double max{0.0};

  // Dynamic state
  Dataflow::timestamp_t last_tick{-1.0};
  bool triggered{false};

  // Control/Element virtuals
  void pre_tick(const TickData& td) override;
  void enable() override;

  // Apply max
  void limit();

public:
  using Control::Control;

  // Getters/Setters
  double get_x() const { return p.x; }
  void set_x(double x) { p.x = start_p.x = x; limit(); }
  double get_y() const { return p.y; }
  void set_y(double y) { p.y = start_p.y = y; limit(); }
  double get_z() const { return p.z; }
  void set_z(double z) { p.z = start_p.z = z; limit(); }
  double get_dx() const { return v.x; }
  void set_dx(double dx) { v.x = dx; }
  double get_dy() const { return v.y; }
  void set_dy(double dy) { v.y = dy; }
  double get_dz() const { return v.z; }
  void set_dz(double dz) { v.z = dz; }
  double get_max() const { return max; }
  void set_max(double m) { max = m; limit(); }
  void set_triggered() { triggered = true; }
};

//--------------------------------------------------------------------------
// Limit the velocity
void VelocityControl::limit()
{
  // Check against max magnitude
  if (max != 0)
  {
    coord_t mag = v.length();
    if (mag > max) v *= max/mag;  // Scale equally
  }
}

//--------------------------------------------------------------------------
// Enable (reset)
void VelocityControl::enable()
{
  triggered = false;
  last_tick = -1.0;
  p = start_p;
}

//--------------------------------------------------------------------------
// Tick
void VelocityControl::pre_tick(const TickData& td)
{
  if (wait)
  {
    if (!triggered) return;
    triggered = false;
  }

  if (last_tick >= 0.0)
  {
    auto delta_t = td.t - last_tick;
    auto delta = v * delta_t;
    p += delta;
    send("x", Dataflow::Value{p.x});
    send("y", Dataflow::Value{p.y});
    send("z", Dataflow::Value{p.z});
  }

  last_tick = td.t;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "velocity",
  "Velocity",
  "Apply Cartesian velocity to a Translate",
  "vector",
  {
    { "x", { "Current X co-ordinate", Value::Type::number,
             { &VelocityControl::get_x, &VelocityControl::set_x}, true } },
    { "y", { "Current Y co-ordinate", Value::Type::number,
             { &VelocityControl::get_y, &VelocityControl::set_y}, true } },
    { "z", { "Current Z co-ordinate", Value::Type::number,
             { &VelocityControl::get_z, &VelocityControl::set_z}, true } },
    { "dx", { "X component of velocity", Value::Type::number,
              { &VelocityControl::get_dx, &VelocityControl::set_dx}, true } },
    { "dy", { "Y component of velocity", Value::Type::number,
              { &VelocityControl::get_dy, &VelocityControl::set_dy}, true } },
    { "dz", { "Z component of velocity", Value::Type::number,
              { &VelocityControl::get_dz, &VelocityControl::set_dz}, true } },
    { "max", { "Maximum magnitude of velocity", Value::Type::number,
               { &VelocityControl::get_max, &VelocityControl::set_max },
               true } },
    { "wait",  { "Whether to wait for a trigger", Value::Type::number,
                 &VelocityControl::wait, false } },
    { "trigger", { "Trigger to set value", Value::Type::trigger,
                   &VelocityControl::set_triggered, true } }
  },
  {
    { "x", { "X co-ordinate", "x", Value::Type::number }},
    { "y", { "Y co-ordinate", "y", Value::Type::number }},
    { "z", { "Z co-ordinate", "z", Value::Type::number }}
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(VelocityControl, module)
