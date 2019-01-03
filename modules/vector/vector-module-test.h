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

    graph.attach(&catcher);

    double t = 0.0;
    for(auto i=0; i<nticks; i++, t+=1.0)
      graph.tick(t);
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
