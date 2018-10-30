//==========================================================================
// ViGraph dataflow machine: test-classes.h
//
// Tests for multigraph container structure
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

using namespace std;
using namespace ObTools;
using namespace ViGraph;
using namespace ViGraph::Dataflow;

//==========================================================================
// Test data type
struct TestData: public Data
{
  int n;
  TestData(int _n): n(_n) {}
};

//==========================================================================
// Test Source
class TestSource: public Source
{
public:
  int value = 0;
  bool shutdown_called = false;

  // Construct
  TestSource(const Module *module, const XML::Element& config):
    Element(module, config), Source(module, config)
  {
  }

  // Configure
  void configure(const XML::Element& config) override
  {
    value = config.get_attr_int("value");
  }

  // Generate some data
  void tick(timestamp_t t) override
  {
    send(new TestData((int)t));
  }

  // Shutdown
  void shutdown() override
  {
    shutdown_called = true;
  }
};

Module TestSourceModule
{
  "test-source", "", "", "test",
  {},
  {}, // no inputs
  { "VectorFrame" }
};

Registry::NewFactory<TestSource> TestSourceFactory;

//==========================================================================
// Test Filter
class TestFilter: public Filter
{
public:
  int value;

  // Construct
  TestFilter(const Module *module, const XML::Element& config):
    Element(module, config), Filter(module, config)
  {
  }

  // Configure
  void configure(const XML::Element& config) override
  {
    value = config.get_attr_int("value");
  }

  // Process some data
  void accept(DataPtr data) override
  {
    auto td = data.check<TestData>();
    TestData *tdout = new TestData(td->n * value);
    send(tdout);
  }
};

Module TestFilterModule
{
  "test-filter", "", "", "test",
  {},
  { "VectorFrame" }, // inputs
  { "VectorFrame" }
};

Registry::NewFactory<TestFilter> TestFilterFactory;

//==========================================================================
// Test Sink
class TestSink: public Sink
{
public:
  int received_data = 0;  // Accumulates
  bool pre_tick_called{false};
  bool post_tick_called{false};

  // Construct
  TestSink(const Module *module, const XML::Element& config):
    Sink(module, config)
  {
  }

  // Process some data
  void accept(DataPtr data) override
  {
    auto td = data.check<TestData>();
    received_data += td->n;
  }

  // Notify of tick
  void pre_tick(timestamp_t) override
  {
    pre_tick_called = true;
  }
  void post_tick(timestamp_t) override
  {
    post_tick_called = true;
  }
};

Module TestSinkModule
{
  "test-sink", "", "", "test",
  {},
  { "VectorFrame" }, // inputs
  {}
};

Registry::NewFactory<TestSink> TestSinkFactory;

//==========================================================================
// Global engine - for registry only, we create our own graphs
Engine engine;

void register_elements()
{
  engine.element_registry.add(TestSourceModule, TestSourceFactory);
  engine.element_registry.add(TestFilterModule, TestFilterFactory);
  engine.element_registry.add(TestSinkModule, TestSinkFactory);
}

// Try to construct a graph and capture any error
inline void construct_graph(const string& xml, Dataflow::Graph& graph)
{
  XML::Configuration config;
  ASSERT_TRUE(config.read_text(xml));

  try
  {
    register_elements();
    graph.configure(config.get_root());
  }
  catch (runtime_error e)
  {
    FAIL() << "Can't create graph: " << e.what() << endl;
    return;
  }
}

// Ensure a graph construction fails
inline void construct_graph_should_fail(const string& xml,
                                        Dataflow::Graph& graph)
{
  XML::Configuration config;
  ASSERT_TRUE(config.read_text(xml));

  try
  {
    register_elements();
    graph.configure(config.get_root());
  }
  catch (runtime_error e)
  {
    return;
  }

  FAIL() << "Graph contruction should have failed\n";
}

// Try to construct a multigraph and capture any error
inline void construct_multigraph(const string& xml, Dataflow::MultiGraph& graph)
{
  XML::Configuration config;
  ASSERT_TRUE(config.read_text(xml));

  try
  {
    register_elements();
    graph.configure(config.get_root());
  }
  catch (runtime_error e)
  {
    FAIL() << "Can't create multigraph: " << e.what() << endl;
    return;
  }
}
