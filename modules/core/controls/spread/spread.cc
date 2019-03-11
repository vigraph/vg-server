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
  vector<Value> values;
  set<Value> capturing;
  unsigned pos = 0;
  unique_ptr<Value> last_on;
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
  Element(module, config), Control(module, config)
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
  last_on.reset();
}

//--------------------------------------------------------------------------
// Set a control property
void SpreadControl::set_property(const string& prop,
                                 const SetParams& sp)
{
  if (prop == "trigger")
  {
    if (last_on)
    {
      send("off", *last_on);
      last_on.reset();
    }
    if (latch || !capturing.empty())
    {
      last_on.reset(new Value{values[pos]});
      send("on", values[pos]);
      if (++pos >= values.size())
        pos = 0;
    }
  }
  else if (prop == "on")
  {
    if (capturing.empty())
    {
      values.clear();
      pos = 0;
    }
    capturing.insert(sp.v);
    values.push_back(sp.v);
  }
  else if (prop == "off")
  {
    capturing.erase(sp.v);
    if (!latch)
    {
      for (auto i = 0u; i < values.size(); ++i)
      {
        if (values[i] == sp.v)
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
    { "trigger", { "Note trigger", Value::Type::trigger, "@trigger", true }},
    { "on", { "Note on", Value::Type::number, "@on", true }},
    { "off", { "Note off", Value::Type::number, "@off", true }},
    { "latch", { "Latch", Value::Type::boolean, "@latch", true }},
  },
  {
    { "on", { "Note on", "on", Value::Type::number }},
    { "off", { "Note off", "off", Value::Type::number }},
  }

};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(SpreadControl, module)
