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
#include <dlfcn.h>

// Module vg_init function interface
typedef bool vg_init_fn_t(Log::Channel&, Dataflow::Engine&);

namespace ViGraph { namespace Module { namespace Test {

// Module loader
class ModuleLoader
{
public:
  Dataflow::Engine engine;

  ModuleLoader()
  {
    engine.add_default_section("core");
  }

  void load(const string& path)
  {
    void *dl_handle = dlopen(path.c_str(), RTLD_NOW);
    ASSERT_FALSE(!dl_handle) << "Can't load " << path << endl;
    void *fn = dlsym(dl_handle, "vg_init");
    ASSERT_FALSE(!fn);

    ASSERT_TRUE(reinterpret_cast<vg_init_fn_t *>(fn)(Log::logger, engine));
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
  Element(&module_data, XML::Element("test")), prop_type(_prop_type) {}

  // Test helpers
  bool got(const string& prop) { return properties.find(prop)!=properties.end(); }
  const Value& get(const string& prop) { return properties[prop]; }
};

// Control tester - creates a control graph from XML, attaches the test
// control to it and looks at its set output
class ControlTester
{
  ModuleLoader& loader;
  Graph graph;
  int id_serial{0};

  void test(list<Element *>& elements, int nticks)
  {
    for(auto e: elements) graph.add(e);
    graph.add(target);

    for(auto e: elements) graph.connect(e);
    graph.connect(target);

    graph.generate_topological_order();

    for(auto i=0; i<nticks; i++)
    {
      const auto td = TickData(i, i, Time::Duration{1});
      graph.pre_tick(td);
      graph.tick(td);
      graph.post_tick(td);
    }
  }

 public:
  TestTarget *target;

  // Generic test with list of XMLs
  void test(const list<string>& xmls, int nticks=1)
  {
    list<Element *> elements;
    for(const auto& xml: xmls)
    {
      try
      {
        XML::Configuration config;
        ASSERT_TRUE(config.read_text(xml));
        const auto& xe = config.get_root();
        Element *e = loader.engine.create(xe.name, xe);
        if (!e) throw runtime_error("Can't create element "+xe.name);
        if (e->id.empty()) e->id = xe.name + Text::itos(++id_serial);
        e->graph = &graph;
        e->configure(File::Directory("."), xe);
        elements.push_back(e);
      }
      catch (runtime_error e)
      {
        FAIL() << "Can't create graph: " << e.what() << endl;
        return;
      }
    }

    test(elements, nticks);
  }

  // Basic test with 1 XMLs
  void test(const string& xml, int nticks=1)
  {
    list<string> xmls;
    xmls.push_back(xml);
    test(xmls, nticks);
  }

  // Test with 2 XMLs
  void test(const string& xml1, const string& xml2, int nticks=1)
  {
    list<string> xmls;
    xmls.push_back(xml1);
    xmls.push_back(xml2);
    test(xmls, nticks);
  }

  // Test with 3 XMLs
  void test(const string& xml1, const string& xml2,
            const string& xml3, int nticks=1)
  {
    list<string> xmls;
    xmls.push_back(xml1);
    xmls.push_back(xml2);
    xmls.push_back(xml3);
    test(xmls, nticks);
  }

  ControlTester(ModuleLoader& _loader,
                Value::Type prop_type = Value::Type::number):
  loader(_loader), graph(loader.engine), target(new TestTarget(prop_type)) {}
};

}}} // namespaces

using namespace ViGraph::Module::Test;

#endif // !__VIGRAPH_MODULE_TEST_H
