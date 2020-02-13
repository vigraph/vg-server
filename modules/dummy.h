//==========================================================================
// Dummy templates
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_MODULE_DUMMY_H
#define __VIGRAPH_MODULE_DUMMY_H

#include "ot-log.h"
#include "vg-dataflow.h"

using namespace std;
using namespace ObTools;
using namespace ViGraph;
using namespace ViGraph::Dataflow;

const auto default_sample_rate = 1000;

//==========================================================================
// Dummy out template
template<typename T>
class DummyOut: public SimpleElement
{
private:
  void setup(const SetupContext& context) override
  {
    SimpleElement::setup(context);
    input.set_sample_rate(sample_rate);
  }

public:
  using SimpleElement::SimpleElement;

  Setting<Number> sample_rate{default_sample_rate};
  Input<T> input{};
};

#endif // !__VIGRAPH_MODULE_DUMMY_H
