//==========================================================================
// Gate module
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../core-module.h"
#include "../../gate.h"

namespace {

class NumberGate: public Gate<Number>
{
private:
  // Clone
  NumberGate *create_clone() const override
  {
    return new NumberGate{module};
  }
public:
  using Gate::Gate;
};

const Dataflow::SimpleModule module =
{
  "gate",
  "Gate",
  "core",
  {},
  {
    { "input",    &NumberGate::input },
    { "control",  &NumberGate::control },
    { "open",     &NumberGate::open },
    { "close",    &NumberGate::close },
  },
  {
    { "output",   &NumberGate::output },
  }
};

} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(NumberGate, module)
