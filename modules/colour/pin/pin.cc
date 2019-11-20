//==========================================================================
// ViGraph dataflow module: colour/pin/pin.cc
//
// Colour valued pin module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../colour-module.h"

namespace {

class ColourPin: public Pin<Colour::RGB>
{
private:
  // Clone
  ColourPin *create_clone() const override
  {
    return new ColourPin{module};
  }
public:
  using Pin::Pin;
};

Dataflow::SimpleModule module
{
  "pin",
  "Colour Pin",
  "colour",
  {},
  {
    { "input",  &ColourPin::input },
  },
  {
    { "output", &ColourPin::output },
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(ColourPin, module)
