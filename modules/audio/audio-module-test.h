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

// Generator class - creates graph from XML, ticks it and captures
// first fragment
class FragmentGenerator
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
    uint64_t n = 0;
    for(auto i=0; i<nticks; i++, t+=1.0, ++n)
    {
      const auto td = TickData{t, n, Time::Duration{1}};
      graph.pre_tick(td);
      graph.tick(td);
      graph.post_tick(td);
    }
  }

public:
  FragmentGenerator(const string& xml, ModuleLoader& _loader, int _nticks = 3):
    loader(_loader), nticks(_nticks)
  { generate(xml); }
  Fragment *get_fragment() { return catcher.last_fragment.get(); }
};

// Generator class which expects to fail creation
class BadFragmentGenerator
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
  BadFragmentGenerator(const string& xml, ModuleLoader& _loader):
    loader(_loader)
  { generate(xml); }
};

}}} // namespaces

#endif // !__VIGRAPH_AUDIO_MODULE_TEST_H
