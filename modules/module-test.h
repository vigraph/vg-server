//==========================================================================
// ViGraph dataflow modules: module-test.h
//
// Support classes for graph element tests
//
// Copyright (c) 2017-2018 Paul Clark.  All rights reserved
//==========================================================================

#ifndef __VIGRAPH_MODULE_TEST_H
#define __VIGRAPH_MODULE_TEST_H

#include "module.h"
#include <gtest/gtest.h>
#include "ot-lib.h"

// Module vg_init function interface
typedef bool vg_init_fn_t(Log::Channel&, Dataflow::Engine&);

namespace ViGraph { namespace Module { namespace Test {

// Module loader
class ModuleLoader
{
private:
  map<string, unique_ptr<ObTools::Lib::Library>> libs;

public:
  Dataflow::Engine engine;

  ModuleLoader()
  {
    add_default_section("core");
  }

  void add_default_section(const string& section)
  {
    engine.add_default_section(section);
  }

  void load(const string& path)
  {
    ASSERT_TRUE(libs.find(path) == libs.end())
      << "Library already loaded: " << path << endl;
    auto lib = make_unique<ObTools::Lib::Library>(path);
    ASSERT_FALSE(!*lib) << "Can't load " << path << endl;
    auto fn = lib->get_function<vg_init_fn_t *>("vg_init");
    ASSERT_TRUE(fn);
    ASSERT_TRUE(fn(Log::logger, engine));
    libs.emplace(path, lib.release());
  }
};

// Test control - accepts any property and remembers it
template<typename T>
class TestTarget: public Element
{
private:
  struct Module module_data =
  {
    "test",
    "Test",
    "test",
    {},
    {
      { "input", &TestTarget::input },
    },
    {}
  };

public:
  Input<T> input;
  vector<T> capture;

  // Construct
  TestTarget():
    Element(module_data)
  {
    id = "test";
  }

  void tick(const TickData&) override
  {
    capture = input.get_buffer();
  }
};

// Graph constructor
template<typename T>
class GraphTester
{
  ModuleLoader& loader;
  int id_serial{0};
  double sample_rate = 50;

 public:
  Graph graph;
  TestTarget<T> *target = nullptr;

  // Add an element
  Element& add(const string& name)
  {
    Element *e = loader.engine.create(name);
    if (!e) throw runtime_error("Can't create element "+name);
    if (e->id.empty()) e->id = name + Text::itos(++id_serial);
    graph.add(e);
    e->graph = &graph;
    return *e;
  }

  // Run test
  void run(int nticks = 1)
  {
    graph.setup();
    graph.set_sample_rate(sample_rate);

    for(auto i=0; i<nticks; i++)
    {
      graph.tick(TickData(i, sample_rate, sample_rate));
    }
  }

  GraphTester(ModuleLoader& _loader,
              double _sample_rate = 50):
  loader(_loader), sample_rate(_sample_rate), graph(loader.engine)
  {
    target = new TestTarget<T>{};
    graph.add(target);
  }

  bool capture_from(Element& element, const string& output)
  {
    return element.connect(output, *target, "input");
  }

  const vector<T>& get_output()
  {
    return target->capture;
  }
};

}}} // namespaces

using namespace ViGraph::Module::Test;

#endif // !__VIGRAPH_MODULE_TEST_H
