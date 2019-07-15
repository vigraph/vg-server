//==========================================================================
// ViGraph dataflow module: controls/spread/spread.cc
//
// Animation control to spread a value to a control setting
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>

namespace {

//==========================================================================
// Spread control
class SpreadControl: public Dataflow::Control
{
  vector<double> values;
  set<double> capturing;
  unsigned index = 0;
  unique_ptr<Value> last_sent;
  bool enabled = true;

  // Reset state
  void reset();
  void cleardown();

  // Control/Element virtuals
  void enable() override { reset(); }

public:
  bool latch = false;
  double value = 0.0;
  using Control::Control;

  void start_capture();
  void stop_capture();
  void next();

  // Enable/disable operation
  void enable_it() { reset(); enabled = true; }
  void disable_it() { cleardown(); enabled = false; }

  // Set latch
  void set_latch(bool v) { latch = v; if (!latch) cleardown(); }
};

//--------------------------------------------------------------------------
// Reset state of the control
void SpreadControl::reset()
{
  values.clear();
  capturing.clear();
  index = 0;
  last_sent.reset();
}

//--------------------------------------------------------------------------
// Send a clear for all current values and reset
void SpreadControl::cleardown()
{
  for(const auto v: values)
  {
    send("output", v);
    trigger("clear");
  }

  reset();
}

//--------------------------------------------------------------------------
// Capture current value
void SpreadControl::start_capture()
{
  if (!enabled)
  {
    // Pass trigger straight through
    send("output", value);
    trigger("trigger");
    return;
  }

  if (capturing.empty())
  {
    values.clear();
    index = 0;
  }
  capturing.insert(value);
  values.push_back(value);
}

//--------------------------------------------------------------------------
// Stop capturing current value
void SpreadControl::stop_capture()
{
  if (!enabled)
  {
    // Pass clear straight through
    send("output", value);
    trigger("clear");
    return;
  }

  capturing.erase(value);
  if (!latch)
  {
    for (auto i = 0u; i < values.size(); ++i)
    {
      if (values[i] == value)
      {
        values.erase(values.begin() + i);
        if (index > i)
          --index;
        if (index > values.size())
          index = 0;
        break;
      }
    }
  }
}

//--------------------------------------------------------------------------
// Trigger next value
void SpreadControl::next()
{
  if (!enabled) return;

  if (last_sent)
  {
    send("output", *last_sent);
    trigger("clear");
    last_sent.reset();
  }
  if ((latch || !capturing.empty()) && !values.empty())
  {
    last_sent.reset(new Value{values[index]});
    send("output", *last_sent);
    trigger("trigger");
    if (++index >= values.size())
      index = 0;
  }
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "spread",
  "Spread",
  "Spread a collection of values over time",
  "core",
  {
    { "value", { "A value", Value::Type::number,
                 &SpreadControl::value, true } },
    { "trigger", { "Trigger value", Value::Type::trigger,
                   &SpreadControl::start_capture, true } },
    { "clear", { "Clear value", Value::Type::trigger,
                 &SpreadControl::stop_capture, true } },
    { "next", { "Send next value", Value::Type::trigger,
                &SpreadControl::next, true } },
    { "latch", { "Latch", Value::Type::boolean,
          &SpreadControl::set_latch, true } },
    { "enable", { "Enable spreading", Value::Type::trigger,
                   &SpreadControl::enable_it, true } },
    { "disable", { "Disable spreading (pass through)", Value::Type::trigger,
                   &SpreadControl::disable_it, true } },
  },
  {
    { "output", { "Value", "value", Value::Type::number } },
    { "trigger", { "Trigger value", "trigger", Value::Type::trigger } },
    { "clear", { "Clear value", "clear", Value::Type::trigger } },
  }

};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SpreadControl, module)
