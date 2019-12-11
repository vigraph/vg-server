//==========================================================================
// ViGraph dataflow module: core/number-pin/number-pin.cc
//
// Trigger valued pin module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../core-module.h"

namespace {

class TriggerPin: public Pin<Trigger>
{
private:
  // Clone
  TriggerPin *create_clone() const override
  {
    return new TriggerPin{module};
  }
public:
  using Pin::Pin;
};

Dataflow::SimpleModule module
{
  "trigger-pin",
  "Trigger pin",
  "core",
  {},
  {
    { "input",  &TriggerPin::input },
  },
  {
    { "output", &TriggerPin::output },
  }
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(TriggerPin, module)
