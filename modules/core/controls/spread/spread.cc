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
  Value value;
  vector<Value> values;
  set<Value> capturing;
  unsigned pos = 0;
  unique_ptr<Value> last_sent;
  bool latch = false;

  // Reset state
  void reset();

  // Control/Element virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void enable() override { reset(); }

public:
  // Construct
  SpreadControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
//   <spread offset="0.1" property="x"/>
SpreadControl::SpreadControl(const Module *module, const XML::Element& config):
  Control(module, config)
{
  latch = config.get_attr_bool("latch");
}

//--------------------------------------------------------------------------
// Reset state of the control
void SpreadControl::reset()
{
  values.clear();
  capturing.clear();
  pos = 0;
  last_sent.reset();
}

//--------------------------------------------------------------------------
// Set a control property
void SpreadControl::set_property(const string& prop,
                                 const SetParams& sp)
{
  if (prop == "value")
  {
    value = sp.v;
  }
  else if (prop == "next")
  {
    if (last_sent)
    {
      send("value", *last_sent);
      send("clear", {});
    }
    if (latch || !capturing.empty())
    {
      last_sent.reset(new Value{values[pos]});
      send("value", *last_sent);
      send("trigger", {});
      if (++pos >= values.size())
        pos = 0;
    }
  }
  else if (prop == "trigger")
  {
    if (capturing.empty())
    {
      values.clear();
      pos = 0;
    }
    capturing.insert(value);
    values.push_back(value);
  }
  else if (prop == "clear")
  {
    capturing.erase(value);
    if (!latch)
    {
      for (auto i = 0u; i < values.size(); ++i)
      {
        if (values[i] == value)
        {
          values.erase(values.begin() + i);
          if (pos > i)
            --pos;
          if (pos > values.size())
            pos = 0;
          break;
        }
      }
    }
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
    { "value", { "A value", Value::Type::number, true }},
    { "trigger", { "Trigger value", Value::Type::trigger, true }},
    { "clear", { "Clear value", Value::Type::trigger, true }},
    { "next", { "Send next value", Value::Type::trigger, true }},
    { "latch", { "Latch", Value::Type::boolean, "@latch", true }},
  },
  {
    { "value", { "Value", "value", Value::Type::number }},
    { "trigger", { "Trigger value", "trigger", Value::Type::trigger }},
    { "clear", { "Clear value", "clear", Value::Type::trigger }},
  }

};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SpreadControl, module)
