//==========================================================================
// ViGraph dataflow module: controls/smooth/smooth.cc
//
// Control to smooth a value of a control setting
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"

namespace {

//==========================================================================
// Smooth control
class SmoothControl: public Dataflow::Control
{
  string property;
  double rc = 0.3; // RC time constant

  // Time for this tick interval
  Time::Duration tick_interval;

  // Previous smoothed value
  double value = 0.0;

  // Control/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  Dataflow::Value::Type get_property_type(const string& property) override;
  void pre_tick(const TickData& td) override;

public:
  // Construct
  SmoothControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//   <smooth property="x"/>
SmoothControl::SmoothControl(const Module *module, const XML::Element& config):
  Element(module, config), Control(module, config)
{
  property = config["property"];
  rc = config.get_attr_real("time", rc);
}

//--------------------------------------------------------------------------
// Record tick interval
void SmoothControl::pre_tick(const TickData& td)
{
  tick_interval = td.interval;
}

//--------------------------------------------------------------------------
// Set a control property
void SmoothControl::set_property(const string& prop, const SetParams& sp)
{
  if (prop == property)
  {
    // Smooth the value and pass on
    const auto dt = tick_interval.seconds();
    const auto div = rc + dt;
    if (!div)
      return;
    const auto a =  dt / div;
    value = a * sp.v.d + (1 - a) * value;
    SetParams nsp(Dataflow::Value{value});
    send(nsp);
  }
  else if (prop == "time")
  {
    update_prop(rc, sp);
  }
}

//--------------------------------------------------------------------------
// Get control property types
Dataflow::Value::Type
  SmoothControl::get_property_type(const string& prop)
{
  if (prop == property)
    return Dataflow::Value::Type::number;
  else if (prop == "time")
    return Dataflow::Value::Type::number;
  else
    return Dataflow::Value::Type::invalid;
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "smooth",
  "Smooth",
  "Smooth a control value",
  "core",
  {
    { "property", { "Property to set", Value::Type::text } },
    { "time", { {"RC time constant (seconds)", "0.3"}, Value::Type::number,
                "@time", true } },
  },
  { { "", { "Value output", "", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SmoothControl, module)
