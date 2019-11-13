//==========================================================================
// ViGraph dataflow module: vector/pin/pin.cc
//
// Vector frame valued pin module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"

namespace {

class VectorPin: public Pin<Frame>
{
private:
  // Clone
  VectorPin *create_clone() const override
  {
    return new VectorPin{module};
  }
public:
  using Pin::Pin;
};

Dataflow::SimpleModule module
{
  "pin",
  "Vector Pin",
  "vector",
  {},
  {
    { "input",  &VectorPin::input },
  },
  {
    { "output", &VectorPin::output },
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(VectorPin, module)
