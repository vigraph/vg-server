//==========================================================================
// ViGraph dataflow module: bitmap/switch/switch.cc
//
// Switch module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"
#include "../../switch.h"

template<>
inline Bitmap::Group switch_fade(const Bitmap::Group& value, double factor)
{
  auto g = value;
  for (auto& i: g.items)
    i.rect.fade(factor);
  return g;
}

namespace {

class BitmapSwitch: public Switch<Bitmap::Group>
{
public:
  const static Dataflow::DynamicModule switch_module;

private:
  // Clone
  BitmapSwitch *create_clone() const override
  {
    return new BitmapSwitch{switch_module};
  }
public:
  using Switch::Switch;
};

const Dataflow::DynamicModule BitmapSwitch::switch_module =
{
  "switch",
  "Switch",
  "bitmap",
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

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(BitmapSwitch, BitmapSwitch::switch_module)
