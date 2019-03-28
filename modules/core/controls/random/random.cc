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
public:
  // Configured state
  double min{0.0};
  double max{1.0};
  bool wait{false};

private:
  // Dynamic state
  bool done{false};
  bool triggered{false};

  // Control virtuals
  void enable() override;
  void pre_tick(const TickData& td) override;
  void notify_target_of(Element *, const string& property) override;

public:
  using Control::Control;

  // Getters/Setters
  void set_triggered() { triggered = true; }
};

//--------------------------------------------------------------------------
// Automatically set wait flag if we are the trigger target of something
void RandomControl::notify_target_of(Element *, const string& property)
{
  if (property == "trigger")
    wait = true;
}

//--------------------------------------------------------------------------
// Tick
void RandomControl::enable()
{
  triggered = done = false;
}

//--------------------------------------------------------------------------
// Tick
void RandomControl::pre_tick(const TickData& /*td*/)
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
    { "min", { "Minimum value", Value::Type::number,
               &RandomControl::min, true } },
    { "max", { { "Maximum value", "1.0" }, Value::Type::number,
               &RandomControl::max, true } },
    { "wait",  { "Whether to wait for a trigger", Value::Type::boolean,
                 &RandomControl::wait, false } },
    { "trigger", { "Trigger to set value", Value::Type::trigger,
                   &RandomControl::set_triggered, true } }
  },
  { { "", { "Random value", "", Value::Type::number }}}
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(RandomControl, module)
