//==========================================================================
// ViGraph dataflow module: core/switch/switch.cc
//
// Switch module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../core-module.h"
#include "../../switch.h"

template<>
inline double switch_fade(const double& value, double factor)
{
  return value * factor;
}

namespace {

class NumberSwitch: public Switch<double>
{
public:
  const static Dataflow::DynamicModule switch_module;

private:
  // Clone
  NumberSwitch *create_clone() const override
  {
    return new NumberSwitch{switch_module};
  }
public:
  using Switch::Switch;
};

const Dataflow::DynamicModule NumberSwitch::switch_module =
{
  "switch",
  "Switch",
  "core",
  {
    { "inputs",         &Switch::inputs },
  },
  {
    { "number",         &Switch::number },
    { "fraction",       &Switch::fraction },
    { "next",           &Switch::next },
    { "fade-in-time",   &Switch::fade_in_time },
    { "fade-out-time",  &Switch::fade_out_time },
  },
  {
    { "output",         &Switch::output },
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(NumberSwitch, NumberSwitch::switch_module)
