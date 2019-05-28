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
    graph.setup();
    graph.attach_external(&catcher);
    test(nticks);
  }

  Frame *get_frame() { return catcher.last_frame.get(); }
};

// !!! Remove once XML gone!
// Generator class - creates graph from XML, ticks it and captures
// first frame
class FrameGenerator
{
  Catcher catcher;
  ModuleLoader& loader;
  int nticks;

  void generate(const string& xml)
  {
    XML::Configuration config;
    ASSERT_TRUE(config.read_text(xml));
    loader.engine.add_default_section("vector");

    Graph graph(loader.engine);
    try
    {
      graph.configure(File::Directory("."), config.get_root());
    }
    catch (runtime_error e)
    {
      FAIL() << "Can't create graph: " << e.what() << endl;
      return;
    }

    graph.attach_external(&catcher);

    double t = 0.0;
    uint64_t n = 0;
    for(auto i=0; i<nticks; i++, t+=1.0, ++n)
    {
      graph.pre_tick({t, n, Time::Duration{1}, 1});
      graph.tick({t, n, Time::Duration{1}, 1});
      graph.post_tick({t, n, Time::Duration{1}, 1});
    }
  }

public:
  FrameGenerator(const string& xml, ModuleLoader& _loader, int _nticks = 3):
    loader(_loader), nticks(_nticks)
  { generate(xml); }
  Frame *get_frame() { return catcher.last_frame.get(); }
};

// Generator class which expects to fail creation
class BadFrameGenerator
{
  ModuleLoader& loader;

  void generate(const string& xml)
  {
    XML::Configuration config;
    ASSERT_TRUE(config.read_text(xml));

    loader.engine.add_default_section("vector");
    Graph graph(loader.engine);
    try
    {
      graph.configure(File::Directory("."), config.get_root());
    }
    catch (runtime_error e)
    {
      return;
    }

    FAIL() << "Graph creation should have failed";
  }

public:
  BadFrameGenerator(const string& xml, ModuleLoader& _loader):
    loader(_loader)
  { generate(xml); }
};

}}} // namespaces

#endif // !__VIGRAPH_MODULE_H
