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

// Test source - outputs given buffer
template<typename T>
class TestSource: public SimpleElement
{
private:
  SimpleModule module_data =
  {
    "test-source",
    "Test Source",
    "test",
    {},
    {},
    {
      { "output", &TestSource::output },
    }
  };

  TestSource<T> *create_clone() const override
  {
    return new TestSource<T>{data, get_id()};
  }

  vector<T> data;
  decltype(data.size()) pos = 0;

public:
  Output<T> output;

  // Construct
  TestSource(const vector<T>& _data, const string& id):
    SimpleElement(module_data), data{_data}
  {
    set_id(id);
  }

  void tick(const TickData& td) override
  {
    sample_iterate(td.nsamples, {}, {}, tie(output),
                   [&](double& o)
    {
      if (data.empty())
        o = {};
      else
      {
        o = data[pos++];
        if (pos >= data.size())
          pos = 0;
      }
    });
  }
};

// Test control - accepts any property and remembers it
template<typename T>
class TestTarget: public SimpleElement
{
private:
  SimpleModule module_data =
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

  TestTarget<T> *create_clone() const override
  {
    return new TestTarget<T>{};
  }

public:
  Input<T> input;
  vector<T> capture;

  // Construct
  TestTarget():
    SimpleElement(module_data)
  {
    set_id("test");
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
  TestTarget<T> *target = nullptr;

  // Add an element
  GraphElement& add(const string& name)
  {
    const auto id = name + Text::itos(++id_serial);
    auto e = loader.engine.create(name, id);
    if (!e) throw runtime_error("Can't create element "+name);
    loader.engine.get_graph().add(e);
    return *e;
  }

  // Add a source
  template<typename U>
  GraphElement& add_source(const vector<U>& data)
  {
    const auto id = "test-source" + Text::itos(++id_serial);
    auto e = new TestSource<T>{data, id};
    loader.engine.get_graph().add(e);
    return *e;
  }

  // Run test
  void run(int nticks = 1)
  {
    auto& graph = loader.engine.get_graph();
    graph.setup();
    loader.engine.set_tick_interval(Time::Duration{1});
    loader.engine.set_sample_rate(sample_rate);
    loader.engine.reset();
    loader.engine.update_elements();

    for(auto i=0; i<nticks; i++)
    {
      loader.engine.tick(i);
    }
  }

  GraphTester(ModuleLoader& _loader,
              double _sample_rate = 50):
  loader(_loader), sample_rate(_sample_rate)
  {
    target = new TestTarget<T>{};
    loader.engine.get_graph().add(target);
  }

  bool capture_from(GraphElement& element, const string& output)
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
