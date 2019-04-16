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
  double get_value() const { return value; }
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
                 { &CompareControl::get_value, &CompareControl::set_value },
                 true } },
    { "min", { "Minimum value", Value::Type::number,
               &CompareControl::min, true } },
    { "max", { "Maximum value", Value::Type::number,
                 &CompareControl::max, true } },
    { "on-change", { "Send triggers only on change",
                       Value::Type::boolean,
                       &CompareControl::on_change, true } }
  },
  {
    { "trigger", { "Match", "trigger", Value::Type::trigger }},
    { "clear", { "No match", "clear", Value::Type::trigger }}
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(CompareControl, module)
