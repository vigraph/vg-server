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

class BitmapSwitch: public FadeableSwitch<Bitmap::Group>
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
  using FadeableSwitch::FadeableSwitch;
};

const Dataflow::DynamicModule BitmapSwitch::switch_module =
{
  "switch",
  "Switch",
  "bitmap",
  {
    { "inputs",         &BitmapSwitch::inputs },
  },
  {
    { "number",         &BitmapSwitch::number },
    { "fraction",       &BitmapSwitch::fraction },
    { "next",           &BitmapSwitch::next },
    { "fade-in-time",   &BitmapSwitch::fade_in_time },
    { "fade-out-time",  &BitmapSwitch::fade_out_time },
  },
  {
    { "output",         &BitmapSwitch::output },
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(BitmapSwitch, BitmapSwitch::switch_module)
