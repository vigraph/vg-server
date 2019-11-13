//==========================================================================
// ViGraph dataflow module: core/number-switch/number-switch.cc
//
// Number valued switch module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../core-module.h"
#include "../../switch.h"

namespace {

class NumberSwitch: public Switch<double>
{
private:
  // Clone
  NumberSwitch *create_clone() const override
  {
    return new NumberSwitch{module};
  }
public:
  using Switch::Switch;
};

Dataflow::DynamicModule module
{
  "number-switch",
  "Number Switch",
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

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(NumberSwitch, module)
