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
private:
  // Previous smoothed value
  double last_value = 0.0;
  bool updated = false;

  // Control/Element virtuals
  void update() override { updated = true; }
  void pre_tick(const TickData& td) override;

public:
  double rc = 0.3; // RC time constant
  double value = 0.0;

  // Construct
  using Control::Control;
};

//--------------------------------------------------------------------------
// Record tick interval
void SmoothControl::pre_tick(const TickData& td)
{
  if (updated)
  {
    // Smooth the value and pass on
    const auto dt = td.interval.seconds();
    const auto div = rc + dt;
    if (div)
    {
      const auto a =  dt / div;
      last_value = a * value + (1 - a) * last_value;
    }
    else
    {
      last_value = value;
    }
    send(Dataflow::Value{last_value});
    updated = false;
  }
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
    { "value", { "Value to smooth", Value::Type::number,
                 &SmoothControl::value, true } },
    { "time", { {"RC time constant (seconds)", "0.3"}, Value::Type::number,
                &SmoothControl::rc, true } },
  },
  { { "value", { "Value output", "value", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SmoothControl, module)
