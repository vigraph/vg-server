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
inline Number switch_fade(const Number& value, Number factor)
{
  return value * factor;
}

namespace {

class NumberSwitch: public FadeableSwitch<Number>
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
  using FadeableSwitch::FadeableSwitch;
};

const Dataflow::DynamicModule NumberSwitch::switch_module =
{
  "switch",
  "Switch",
  "core",
  {
    { "inputs",         &NumberSwitch::inputs },
  },
  {
    { "number",         &NumberSwitch::number },
    { "fraction",       &NumberSwitch::fraction },
    { "next",           &NumberSwitch::next },
    { "fade-in-time",   &NumberSwitch::fade_in_time },
    { "fade-out-time",  &NumberSwitch::fade_out_time },
  },
  {
    { "output",         &NumberSwitch::output },
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(NumberSwitch, NumberSwitch::switch_module)
