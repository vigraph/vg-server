//==========================================================================
// ViGraph dataflow module: vector/dummy-out/dummy-out.cc
//
// Vector frame valued dummy-out module
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include "../../dummy.h"

namespace {

class VectorDummyOut: public DummyOut<Frame>
{
private:
  // Clone
  VectorDummyOut *create_clone() const override
  {
    return new VectorDummyOut{module};
  }
public:
  using DummyOut::DummyOut;
};

Dataflow::SimpleModule module
{
  "dummy-out",
  "Vector Dummy Out",
  "vector",
  {
    { "sample-rate",  &VectorDummyOut::sample_rate },
  },
  {
    { "input",        &VectorDummyOut::input },
  },
  {},
};


} // anon

VIGRAPH_ENGINE_ELEMENT_MODULE_INIT(VectorDummyOut, module)
