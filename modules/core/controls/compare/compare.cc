//==========================================================================
// ViGraph dataflow module: controls/compare/compare.cc
//
// Animation control to compare a control setting into a fixed range
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Compare control
class CompareControl: public Dataflow::Control
{
public:
  double value{0.0};
  double min = 0.0;
  double max = 1.0;
  bool on_change = false;

private:
  bool last_result = false;

  // Control/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;

public:
  // Construct
  CompareControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//   <compare min="-0.5" max="0.5" .../>
CompareControl::CompareControl(const Module *module, const XML::Element& config):
  Control(module, config)
{
  min = config.get_attr_real("min", min);
  max = config.get_attr_real("max", max);
  on_change = config.get_attr_bool("on-change", on_change);
}

//--------------------------------------------------------------------------
// Set a control property
void CompareControl::set_property(const string& prop,
                               const SetParams& sp)
{
  if (prop != "value") return;

  if (sp.v.d > max || sp.v.d < min)
  {
    if (!on_change || last_result)
    {
      send("clear", Dataflow::Value{});
      last_result = false;
    }
  }
  else
  {
    if (!on_change || !last_result)
    {
      send("trigger", Dataflow::Value{});
      last_result = true;
    }
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "compare",
  "Compare",
  "Compare a value",
  "core",
  {
    { "value", { "Value input", Value::Type::number,
          static_cast<double Element::*>(&CompareControl::value), true } },
    // !!! Make these settable too!
    { "min", { "Minimum value", Value::Type::number,
          static_cast<double Element::*>(&CompareControl::min) } },
    { "max", { { "Maximum value", "1.0" }, Value::Type::number,
          static_cast<double Element::*>(&CompareControl::max) } },
    { "on-change", { { "Send triggers only on change", "false" },
          Value::Type::boolean,
            static_cast<bool Element::*>(&CompareControl::on_change) } },
  },
  {
    { "trigger", { "Match", "trigger", Value::Type::trigger }},
    { "clear", { "No match", "clear", Value::Type::trigger }},
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(CompareControl, module)
