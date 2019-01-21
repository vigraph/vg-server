//==========================================================================
// ViGraph dataflow module: controls/random/random.cc
//
// Control to randomise properties on other elements
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module.h"
#include <cmath>
#include <stdlib.h>

namespace {

//==========================================================================
// Random control
class RandomControl: public Dataflow::Control
{
  // Configured state
  double min{0.0};
  double max{0.0};
  bool wait{false};

  // Dynamic state
  bool done{false};
  bool triggered{false};

  // Control virtuals
  void set_property(const string& property, const SetParams& sp) override;
  void tick(const TickData& td) override;
  // Automatically set wait flag if we are the target of something
  void notify_target_of(Element *) override { wait = true; }

public:
  // Construct
  RandomControl(const Module *module, const XML::Element& config);
};

//--------------------------------------------------------------------------
// Construct from XML
// <random min="0" max="1"
//      type="{real|integer|boolean}"
//      property="..."/>
RandomControl::RandomControl(const Module *module, const XML::Element& config):
  Element(module, config), Control(module, config)
{
  min = config.get_attr_real("min", 0.0);
  max = config.get_attr_real("max", 1.0);
  wait = config.get_attr_bool("wait");
}

//--------------------------------------------------------------------------
// Set a control property
void RandomControl::set_property(const string& property, const SetParams& sp)
{
  if (property == "trigger")
    triggered = true;
  else if (property == "min")
    update_prop(min, sp);
  else if (property == "max")
    update_prop(max, sp);
}

//--------------------------------------------------------------------------
// Tick
void RandomControl::tick(const TickData& /*td*/)
{
  if (wait)
  {
    if (!triggered) return;
    triggered = false;
  }
  else
  {
    // Only run once
    if (done) return;
    done = true;
  }

  // We're good - do it...
  double val = min + (double)rand()/RAND_MAX*(max-min);
  SetParams sp(Dataflow::Value{val});
  send(sp);
}

//--------------------------------------------------------------------------
// Module definition
Dataflow::Module module
{
  "random",
  "Random",
  "Set a random value on another element",
  "core",
  {
    { "min", { "Minimum value", Value::Type::number, true } },
    { "max", { { "Maximum value", "1.0" }, Value::Type::number, true } },
    { "wait",  { "Whether to wait for a trigger", Value::Type::number } },
    { "trigger", { "Trigger to set value", Value::Type::trigger, true } }
  },
  { { "", { "Random value", "", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(RandomControl, module)
