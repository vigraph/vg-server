//==========================================================================
// ViGraph dataflow module: bitmap/pin/pin.cc
//
// Bitmap group valued pin module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"

namespace {

class BitmapPin: public Pin<Bitmap::Group>
{
private:
  // Clone
  BitmapPin *create_clone() const override
  {
    return new BitmapPin{module};
  }
public:
  using Pin::Pin;
};

Dataflow::SimpleModule module
{
  "pin",
  "Bitmap Pin",
  "bitmap",
  {},
  {
    { "input",  &BitmapPin::input },
  },
  {
    { "output", &BitmapPin::output },
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(BitmapPin, module)
