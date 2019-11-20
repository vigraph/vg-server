//==========================================================================
// ViGraph dataflow module: colour/switch/switch.cc
//
// Switch module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../colour-module.h"
#include "../../switch.h"

template<>
inline Colour::RGB switch_fade(const Colour::RGB& c, double factor)
{
  Colour::RGB r = c;
  r.fade(factor);
  return r;
}

namespace {

class ColourSwitch: public Switch<Colour::RGB>
{
public:
  const static Dataflow::DynamicModule switch_module;

private:
  // Clone
  ColourSwitch *create_clone() const override
  {
    return new ColourSwitch{switch_module};
  }
public:
  using Switch::Switch;
};

const Dataflow::DynamicModule ColourSwitch::switch_module =
{
  "switch",
  "Switch",
  "colour",
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

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ColourSwitch, ColourSwitch::switch_module)
