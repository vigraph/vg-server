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

Module TestSourceModule
{
  "test-source", "", "test",
  {},
  {},
  { {"output", &TestSource::output} },
};

Registry::NewFactory<TestSource> TestSourceFactory;

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

Module TestFilterModule
{
  "test-filter", "", "test",
  { { "value", &TestFilter::value } },
  { { "input", &TestFilter::input } },
  { { "output", &TestFilter::output } },
};

Registry::NewFactory<TestFilter> TestFilterFactory;

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

Module TestSinkModule
{
  "test-sink", "", "test",
  {},
  { { "input", &TestSink::input } }, // inputs
  {}
};

Registry::NewFactory<TestSink> TestSinkFactory;

//==========================================================================
// Test graph - sugars setters
class TestGraph: public Graph
{
  int id_serial{0};

 public:
  using Graph::Graph;

  // Add an element
  Element& add(const string& name, const string& id = "")
  {
    Element *e = get_engine().create(name);
    if (!e) throw runtime_error("Can't create element "+name);
    if (id.empty())
      e->id = name + Text::itos(++id_serial);
    else
      e->id = id;
    Graph::add(e);
    e->graph = this;
    return *e;
  }

  // Get an element of a given type, nullptr if not found or wrong type
  template <typename T> T *get(const string& name)
  {
    Element *el = get_element(name);
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
    engine.element_registry.add(TestSourceModule, TestSourceFactory);
    engine.element_registry.add(TestFilterModule, TestFilterFactory);
    engine.element_registry.add(TestSinkModule, TestSinkFactory);
  }
};
