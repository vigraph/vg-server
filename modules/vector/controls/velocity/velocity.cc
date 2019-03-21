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
  Vector v;   // Velocity
  Vector p;   // Position
  Vector start_p;
  double max{0.0};
  bool wait{false};

  // Dynamic state
  Dataflow::timestamp_t last_tick{-1.0};
  bool triggered{false};

  // Control/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void pre_tick(const TickData& td) override;
  void enable() override;

public:
  // Construct
  VelocityControl(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
// <velocity x="1" y="1" z="1"
//           dx="0.1" dy="0.1" dz="0.0"
//           max = "0.5" wait="false"
//           property-x="x" property-y="y" property-z="z"/>
VelocityControl::VelocityControl(const Dataflow::Module *module,
                                 const XML::Element& config):
  Control(module, config)
{
  start_p.x = config.get_attr_real("x");
  start_p.y = config.get_attr_real("y");
  start_p.z = config.get_attr_real("z");
  p = start_p;
  v.x = config.get_attr_real("dx");
  v.y = config.get_attr_real("dy");
  v.z = config.get_attr_real("dz");
  max = config.get_attr_real("max");
  wait = config.get_attr_bool("wait");
}

//--------------------------------------------------------------------------
// Set a control property
void VelocityControl::set_property(const string& property,
                                   const SetParams& sp)
{
  if (property == "trigger") triggered = true;
  // !!! Need setters to replicate this rest of start_p and p
  else if (property == "x") { update_prop(start_p.x, sp); p.x=start_p.x; }
  else if (property == "y") { update_prop(start_p.y, sp); p.y=start_p.y; }
  else if (property == "z") { update_prop(start_p.z, sp); p.z=start_p.z; }
  else if (property == "dx") update_prop(v.x, sp);
  else if (property == "dy") update_prop(v.y, sp);
  else if (property == "dz") update_prop(v.z, sp);
  else if (property == "max") update_prop(max, sp);

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
    { "x", { "Current X co-ordinate", Value::Type::number, true } },
    { "y", { "Current Y co-ordinate", Value::Type::number, true } },
    { "z", { "Current Z co-ordinate", Value::Type::number, true } },
    { "x", { "X component of velocity", Value::Type::number, true } },
    { "y", { "Y component of velocity", Value::Type::number, true } },
    { "z", { "Z component of velocity", Value::Type::number, true } },
    { "max", { "Maximum magnitude of velocity", Value::Type::number, true } },
    { "wait",  { "Whether to wait for a trigger", Value::Type::number } },
    { "trigger", { "Trigger to set value", Value::Type::trigger, true } }
  },
  {
    { "x", { "X co-ordinate", "x", Value::Type::number }},
    { "y", { "Y co-ordinate", "y", Value::Type::number }},
    { "z", { "Z co-ordinate", "z", Value::Type::number }}
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(VelocityControl, module)
