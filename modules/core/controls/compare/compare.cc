//==========================================================================
// ViGraph dataflow module: controls/compare/compare.cc
//
// Animation control to compare a control setting into a fixed range
//
//   <compare min="-0.5" max="0.5" .../>
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"

namespace {

//==========================================================================
// Compare control
class CompareControl: public Dataflow::Control
{
private:
  bool last_result = false;

public:
  double value{0.0};
  double min = 0.0;
  double max = 1.0;
  bool on_change = false;
  using Control::Control;

  // Property getter/setter
  double get_value() { return value; }
  void set_value(double v);
};

//--------------------------------------------------------------------------
// Setter for the value
void CompareControl::set_value(double v)
{
  if (v > max || v < min)
  {
    if (!on_change || last_result)
    {
      trigger("clear");
      last_result = false;
    }
  }
  else
  {
    if (!on_change || !last_result)
    {
      trigger("trigger");
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
        { static_cast<double (Element::*)()>(&CompareControl::get_value),
          static_cast<void (Element::*)(double)>(&CompareControl::set_value) },
          true } },
    { "min", { "Minimum value", Value::Type::number,
          static_cast<double Element::*>(&CompareControl::min), true } },
    { "max", { { "Maximum value", "1.0" }, Value::Type::number,
          static_cast<double Element::*>(&CompareControl::max), true } },
    { "on-change", { { "Send triggers only on change", "false" },
          Value::Type::boolean,
            static_cast<bool Element::*>(&CompareControl::on_change), true } }
  },
  {
    { "trigger", { "Match", "trigger", Value::Type::trigger }},
    { "clear", { "No match", "clear", Value::Type::trigger }}
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(CompareControl, module)
