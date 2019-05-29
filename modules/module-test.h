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
    ASSERT_TRUE(*lib) << "Can't load " << path << endl;
    auto fn = lib->get_function<vg_init_fn_t *>("vg_init");
    ASSERT_TRUE(fn);
    ASSERT_TRUE(fn(Log::logger, engine));
    libs.emplace(path, lib.release());
  }
};

// Test control - accepts any property and remembers it
class TestTarget: public Element
{
  struct Module module_data =
  {
    "test",
    "Test",
    "Test target",
    "test",
    {
      { "x", { "Test property", Value::Type::number,
               Dataflow::Module::Property::Member(), true } }
    },
    { }
  };

public:
  map<string, Value> properties;
  Value::Type prop_type{Value::Type::number};
  int sets_called{0};

  // Control/Element virtuals
  void set_property(const string& property, const Value& v) override
  { properties[property] = v; sets_called++; }
  Value::Type get_property_type(const string&) override
  { return prop_type; }

public:
  // Construct
 TestTarget(Value::Type _prop_type = Value::Type::number):
  Element(&module_data, XML::Element("test", "id", "test")),
    prop_type(_prop_type) {}

  // Test helpers
  bool got(const string& prop) { return properties.find(prop)!=properties.end(); }
  const Value& get(const string& prop) { return properties[prop]; }
};

// Graph constructor
class GraphTester
{
  ModuleLoader& loader;
  int id_serial{0};

 public:
  struct ElementProxy
  {
    Element *e;
    ElementProxy(Element *_e): e(_e) {}

    // All methods return *this, for chaining
    // Set a property direct with Dataflow::Value
    // Template to avoid ambiguity with JSON version
    template <typename T> const ElementProxy&
    set(const string& name, const T& v) const
    { e->set_property(name, v); return *this; }

    // Set a complex property with JSON
    const ElementProxy& set(const string& name,
                            const JSON::Value& v) const
    { e->set_json(name, v); return *this; }

    // Connect to another element
    const ElementProxy& connect(const string& src_prop,
                                const ElementProxy& dest,
                                const string& dest_prop) const
    {
      JSON::Value json(JSON::Value::ARRAY);
      auto& pj = json.add(JSON::Value(JSON::Value::OBJECT));
      pj.set("element", dest.e->id);
      pj.set("prop", dest_prop);
      e->set_output_json(src_prop, json);
      return *this;
    }

    // Connect to test target
    const ElementProxy& connect_test(const string& src_prop,
                                     const string& dest_prop) const
    {
      JSON::Value json(JSON::Value::ARRAY);
      auto& pj = json.add(JSON::Value(JSON::Value::OBJECT));
      pj.set("element", "test");
      pj.set("prop", dest_prop);
      e->set_output_json(src_prop, json);
      return *this;
    }
  };

  Graph graph;
  TestTarget *target;

  // Add an element
  ElementProxy add(const string& name)
  {
    XML::Element config;  // !!! Until XML goes
    Element *e = loader.engine.create(name, config);
    if (!e) throw runtime_error("Can't create element "+name);
    if (e->id.empty()) e->id = name + Text::itos(++id_serial);
    graph.add(e);
    e->graph = &graph;
    return ElementProxy(e);
  }

  // Run test
  void test(int nticks = 1)
  {
    graph.generate_topological_order();
    graph.set_sample_rate(50);

    for(auto i=0; i<nticks; i++)
    {
      const auto td = TickData(i, i, Time::Duration{1}, 1);
      graph.pre_tick(td);
      graph.tick(td);
      graph.post_tick(td);
    }
  }

  GraphTester(ModuleLoader& _loader,
              Value::Type prop_type = Value::Type::number):
  loader(_loader), graph(loader.engine), target(new TestTarget(prop_type))
  {
    graph.add(target);
  }
};

}}} // namespaces

using namespace ViGraph::Module::Test;

#endif // !__VIGRAPH_MODULE_TEST_H
