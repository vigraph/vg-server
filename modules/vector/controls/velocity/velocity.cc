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
  Vector v;
  double max{0.0};
  bool wait{false};

  // Dynamic state
  Dataflow::timestamp_t last_tick{-1.0};
  bool triggered{false};

  // Control/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void tick(const TickData& td) override;
  void enable() override;

public:
  // Construct
  VelocityControl(const Dataflow::Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
// <velocity x="0.1" y="0.1" z="0.0"
//           max = "0.5" wait="false"
//           property-x="x" property-y="y" property-z="z"/>
VelocityControl::VelocityControl(const Dataflow::Module *module,
                                 const XML::Element& config):
  Element(module, config), Control(module, config)
{
  v.x = config.get_attr_real("x");
  v.y = config.get_attr_real("y");
  v.z = config.get_attr_real("z");
  max = config.get_attr_real("max");
  wait = config.get_attr_bool("wait");
}

//--------------------------------------------------------------------------
// Set a control property
void VelocityControl::set_property(const string& property,
                                   const SetParams& sp)
{
  if (property == "trigger") triggered = true;
  else if (property == "x") update_prop(v.x, sp);
  else if (property == "y") update_prop(v.y, sp);
  else if (property == "z") update_prop(v.z, sp);
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
}

//--------------------------------------------------------------------------
// Tick
void VelocityControl::tick(const TickData& td)
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
    SetParams setx(Dataflow::Value{delta.x}, true);
    send("x", setx);
    SetParams sety(Dataflow::Value{delta.y}, true);
    send("y", sety);
    SetParams setz(Dataflow::Value{delta.z}, true);
    send("z", setz);
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
    { "x", { "X component of velocity", Value::Type::number, true } },
    { "y", { "Y component of velocity", Value::Type::number, true } },
    { "z", { "Z component of velocity", Value::Type::number, true } },
    { "max", { "Maximum magnitude of velocity", Value::Type::number, true } },
    { "wait",  { "Whether to wait for a trigger", Value::Type::number } },
    { "trigger", { "Trigger to set value", Value::Type::trigger, true } }
  },
  {
    { "x", { "X component of velocity", "x", Value::Type::number }},
    { "y", { "Y component of velocity", "y", Value::Type::number }},
    { "z", { "Z component of velocity", "z", Value::Type::number }}
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(VelocityControl, module)
