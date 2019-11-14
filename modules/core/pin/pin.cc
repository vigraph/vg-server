//==========================================================================
// ViGraph dataflow module: core/pin/pin.cc
//
// Number valued pin module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../core-module.h"

namespace {

class NumberPin: public Pin<double>
{
private:
  // Clone
  NumberPin *create_clone() const override
  {
    return new NumberPin{module};
  }
public:
  using Pin::Pin;
};

Dataflow::SimpleModule module
{
  "pin",
  "Pin",
  "core",
  {},
  {
    { "input",  &NumberPin::input },
  },
  {
    { "output", &NumberPin::output },
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(NumberPin, module)
