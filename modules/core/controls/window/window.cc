//==========================================================================
// ViGraph dataflow module: controls/window/window.cc
//
// Animation control to window a control setting into a fixed range
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Window control
class WindowControl: public Dataflow::Control
{
public:
  double min = 0.0;
  double max = 1.0;

  // Construct
  using Control::Control;

  void set_value(double value);
};

//--------------------------------------------------------------------------
// Set value
void WindowControl::set_value(double value)
{
  // Window the value
  if (value > max || value < min)
    return;

  send(value);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "window",
  "Window",
  "Window a value",
  "core",
  {
    { "value", { "Value to check", Value::Type::number,
                 { &WindowControl::set_value }, true } },
    { "min", { "Minimum value", Value::Type::number,
               &WindowControl::min, true } },
    { "max", { "Maximum value", Value::Type::number,
               &WindowControl::max, true } }
  },
  { { "output", { "Value output", "value", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(WindowControl, module)
