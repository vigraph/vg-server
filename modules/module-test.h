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
    return new TestSource<T>{get_id(), data};
  }

  vector<T> data;
  decltype(data.size()) pos = 0;

public:
  Output<T> output;

  // Construct
  TestSource(const string& id, const vector<T>& _data):
    SimpleElement(module_data), data{_data}
  {
    set_id(id);
  }

  void tick(const TickData& td) override
  {
    const auto nsamples = td.samples_in_tick(output.get_sample_rate());
    sample_iterate(nsamples, {}, {}, tie(output),
                   [&](T& o)
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
class TestSink: public SimpleElement
{
private:
  SimpleModule module_data =
  {
    "test-sink",
    "Test",
    "test",
    {},
    {
      { "input", &TestSink::input },
    },
    {}
  };

  TestSink<T> *create_clone() const override
  {
    return new TestSink<T>{get_id(), data, input.get_sample_rate()};
  }

  vector<T>& data;

public:
  Input<T> input;

  // Construct
  TestSink(const string& id, vector<T>& _data, double sample_rate):
    SimpleElement(module_data), data{_data}
  {
    set_id(id);
    input.set_sample_rate(sample_rate);
  }

  void tick(const TickData&) override
  {
    data = input.get_buffer();
  }
};

// Graph constructor
class GraphTester: public ::testing::Test
{
private:
  int id_serial{0};

protected:
  ModuleLoader loader;

public:
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
  template<typename T>
  GraphElement& add_source(const vector<T>& data)
  {
    const auto id = "test-source" + Text::itos(++id_serial);
    auto e = new TestSource<T>{id, data};
    loader.engine.get_graph().add(e);
    return *e;
  }

  // Add a sink
  template<typename T>
  GraphElement& add_sink(vector<T>& data, double sample_rate)
  {
    const auto id = "test-sink" + Text::itos(++id_serial);
    auto e = new TestSink<T>{id, data, sample_rate};
    loader.engine.get_graph().add(e);
    return *e;
  }

  // Run test
  void run(int nticks = 1)
  {
    auto& graph = loader.engine.get_graph();
    graph.setup();
    loader.engine.set_tick_interval(Time::Duration{1});
    loader.engine.reset();
    loader.engine.update_elements();

    for(auto i=0; i<nticks; i++)
    {
      loader.engine.tick(i);
    }
  }
};

}}} // namespaces

using namespace ViGraph::Module::Test;

#endif // !__VIGRAPH_MODULE_TEST_H
