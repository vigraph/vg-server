//==========================================================================
// ViGraph dataflow module: core/containers/pins/number.cc
//
// Basic graph container module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../core-module.h"

namespace {

class NumberPin: public Pin<double>
{
public:
  using Pin::Pin;
};

Dataflow::SimpleModule module
{
  "number-pin",
  "Number Pin",
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
