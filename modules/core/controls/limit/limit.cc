//==========================================================================
// ViGraph dataflow module: controls/limit/limit.cc
//
// Animation control to limit a control setting into a fixed range
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Limit control
class LimitControl: public Dataflow::Control
{
public:
  double min, max;
  double value{0.0};

  // Control/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;

public:
  // Construct
  LimitControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//   <limit value="0.0" min="-0.5" max="0.5" .../>
LimitControl::LimitControl(const Module *module, const XML::Element& config):
  Control(module, config)
{
  min = config.get_attr_real("min", 0.0);
  max = config.get_attr_real("max", 1.0);
  value = config.get_attr_real("value");
}

//--------------------------------------------------------------------------
// Set a control property
void LimitControl::set_property(const string& prop,
                               const SetParams& sp)
{
  if (prop != "value") return;

  // Keep our copy up to date
  update_prop(value, sp);

  // Limit the value
  if (value > max)
    value = max;
  else if (value < min)
    value = min;

  SetParams nsp(Dataflow::Value{value});
  send(nsp);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "limit",
  "Limit",
  "Hard limit on a value",
  "core",
  {
    { "value",
      { "Base value", Value::Type::number,
          static_cast<double Element::*>(&LimitControl::value), true } },
    { "min", { "Minimum value", Value::Type::number,
          static_cast<double Element::*>(&LimitControl::min) } },
    { "max", { { "Maximum value", "1.0" }, Value::Type::number,
          static_cast<double Element::*>(&LimitControl::max) } }
  },
  { { "", { "Value output", "", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(LimitControl, module)
