//==========================================================================
// Gate module
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../trigger-module.h"
#include "../../gate.h"

namespace {

class TriggerGate: public Gate<Trigger>
{
private:
  // Clone
  TriggerGate *create_clone() const override
  {
    return new TriggerGate{module};
  }
public:
  using Gate::Gate;
};

const Dataflow::SimpleModule module =
{
  "gate",
  "Gate",
  "trigger",
  {},
  {
    { "input",    &TriggerGate::input },
    { "control",  &TriggerGate::control },
    { "open",     &TriggerGate::open },
    { "close",    &TriggerGate::close },
  },
  {
    { "output",   &TriggerGate::output },
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(TriggerGate, module)
