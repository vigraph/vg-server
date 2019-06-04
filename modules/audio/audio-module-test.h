//==========================================================================
// ViGraph dataflow modules: audio-module-test.h
//
// Support classes for audio graph element tests
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_AUDIO_MODULE_TEST_H
#define __VIGRAPH_AUDIO_MODULE_TEST_H

#include "../module-test.h"
#include "audio-module.h"

namespace ViGraph { namespace Module { namespace Test {

// Acceptor class to capture output
class Catcher: public Acceptor
{
public:
  shared_ptr<Fragment> last_fragment;

  void accept(DataPtr data) override
  {
    last_fragment = data.check<Fragment>();
  }
};

// Test harness which catches fragments
class FragmentGraphTester: public GraphTester
{
  Catcher catcher;

public:
  FragmentGraphTester(ModuleLoader& _loader):
    GraphTester(_loader, Value::Type::number, 44100)
  {}

  void run(int nticks=1)
  {
    graph.set_send_up_function([this](DataPtr data)
                               { catcher.accept(data); });
    test(nticks);
  }

  Fragment *get_fragment() { return catcher.last_fragment.get(); }
};

}}} // namespaces

#endif // !__VIGRAPH_AUDIO_MODULE_TEST_H
