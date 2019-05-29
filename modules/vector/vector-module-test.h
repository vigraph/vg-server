//==========================================================================
// ViGraph dataflow modules: module-test.h
//
// Support classes for graph element tests
//
// Copyright (c) 2017-2018 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_VECTOR_MODULE_TEST_H
#define __VIGRAPH_VECTOR_MODULE_TEST_H

#include "../module-test.h"
#include "vector-module.h"

namespace ViGraph { namespace Module { namespace Test {

// Acceptor class to capture output
class Catcher: public Acceptor
{
public:
  shared_ptr<Frame> last_frame;

  void accept(DataPtr data) override
  {
    last_frame = data.check<Frame>();
  }
};

// Test harness which catches frames
class FrameGraphTester: public GraphTester
{
  Catcher catcher;

public:
  FrameGraphTester(ModuleLoader& _loader): GraphTester(_loader) {}

  void run(int nticks=1)
  {
    graph.attach_external(&catcher);
    test(nticks);
  }

  Frame *get_frame() { return catcher.last_frame.get(); }
};


}}} // namespaces

#endif // !__VIGRAPH_MODULE_H
