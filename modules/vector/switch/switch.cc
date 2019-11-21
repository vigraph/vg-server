//==========================================================================
// ViGraph dataflow module: vector/switch/switch.cc
//
// Switch module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include "../../switch.h"

template<>
inline Frame switch_fade(const Frame& value, double factor)
{
  auto f = value;
  for (auto& p: f.points)
    p.c.fade(factor);
  return f;
}

namespace {

class VectorSwitch: public Switch<Frame>
{
public:
  const static Dataflow::DynamicModule switch_module;

private:
  // Clone
  VectorSwitch *create_clone() const override
  {
    return new VectorSwitch{switch_module};
  }
public:
  using Switch::Switch;
};

const Dataflow::DynamicModule VectorSwitch::switch_module =
{
  "switch",
  "Switch",
  "vector",
  {
    { "inputs",         &VectorSwitch::inputs },
  },
  {
    { "number",         &VectorSwitch::number },
    { "fraction",       &VectorSwitch::fraction },
    { "next",           &VectorSwitch::next },
    { "fade-in-time",   &VectorSwitch::fade_in_time },
    { "fade-out-time",  &VectorSwitch::fade_out_time },
  },
  {
    { "output",         &VectorSwitch::output },
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(VectorSwitch, VectorSwitch::switch_module)
