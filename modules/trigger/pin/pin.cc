//==========================================================================
// ViGraph dataflow module: trigger/pin/pin.cc
//
// Trigger valued pin module
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../trigger-module.h"

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
  "pin",
  "Trigger pin",
  "trigger",
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
