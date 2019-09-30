//==========================================================================
// ViGraph dataflow machine: test-elements.h
//
// Tests elements and fixtures for graph tests
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

using namespace std;
using namespace ObTools;
using namespace ViGraph;
using namespace ViGraph::Dataflow;

//==========================================================================
// Test Source
class TestSource: public Element
{
public:
  bool shutdown_called = false;
  Output<int> output;
  string *tick_order{nullptr};

  // Construct
  using Element::Element;

  // Generate some data
  void tick(const TickData& td) override
  {
    auto buffer = output.get_buffer();
    buffer.data.push_back(td.timestamp.seconds());
    if (tick_order) *tick_order += id;
  }

  // Shutdown
  void shutdown() override
  {
    shutdown_called = true;
  }
};

SimpleModule TestSourceModule
{
  "test-source", "", "test",
  {},
  {},
  { {"output", &TestSource::output} },
};

Registry::NewFactory<TestSource, SimpleModule>
    TestSourceFactory{TestSourceModule};

//==========================================================================
// Test Filter
class TestFilter: public Element
{
public:
  Setting<int> value;
  Input<int> input;
  Output<int> output;
  string *tick_order{nullptr};

  // Construct
  using Element::Element;

  // Process some data
  void tick(const TickData&) override
  {
    auto& in = input.get_buffer();
    auto out = output.get_buffer();
    for (const auto i: in)
      out.data.push_back(i * value.get());
    if (tick_order) *tick_order += id;
  }

};

SimpleModule TestFilterModule
{
  "test-filter", "", "test",
  { { "value", &TestFilter::value } },
  { { "input", &TestFilter::input } },
  { { "output", &TestFilter::output } },
};

Registry::NewFactory<TestFilter, SimpleModule>
    TestFilterFactory{TestFilterModule};

//==========================================================================
// Test Sink
class TestSink: public Element
{
public:
  MT::Mutex mutex;
  Input<int> input;
  int received_data = 0;  // Accumulates
  string *tick_order{nullptr};

  // Construct
  using Element::Element;

  // Tick
  void tick(const TickData&) override
  {
    MT::Lock lock{mutex};
    const auto& in = input.get_buffer();
    for (const auto i: in)
      received_data += i;
    if (tick_order) *tick_order += id;
  }
};

SimpleModule TestSinkModule
{
  "test-sink", "", "test",
  {},
  { { "input", &TestSink::input } }, // inputs
  {}
};

Registry::NewFactory<TestSink, SimpleModule>
    TestSinkFactory{TestSinkModule};

//==========================================================================
// Test graph - sugars setters
class TestGraph: public Graph
{
  Engine& engine;
  int id_serial{0};

public:
  TestGraph(Engine& _engine, GraphModule& module):
    Graph{module}, engine{_engine}
  {}

  using Graph::add;
  // Add an element
  GraphElement& add(const string& name, string id = "")
  {
    if (id.empty())
      id = name + Text::itos(++id_serial);
    GraphElement *e = engine.create(name, id);
    if (!e) throw runtime_error("Can't create element "+name);
    Graph::add(e);
    return *e;
  }

  // Get an element of a given type, nullptr if not found or wrong type
  template <typename T> T *get(const string& name)
  {
    auto *el = get_element(name);
    if (!el) return nullptr;
    return dynamic_cast<T *>(el);
  }
};

// Test fixture for constructing graphs manually
class GraphTest: public ::testing::Test
{
protected:
  Engine engine;

public:
  void SetUp()
  {
    engine.add_default_section("test");
    engine.element_registry.add("test", "test-source", TestSourceFactory);
    engine.element_registry.add("test", "test-filter", TestFilterFactory);
    engine.element_registry.add("test", "test-sink", TestSinkFactory);
  }
};
