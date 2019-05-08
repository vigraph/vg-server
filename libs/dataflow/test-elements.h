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
  string *tick_order{nullptr};
  string send_tag;
  string receive_tag;

  // Construct
  TestSource(const Module *module, const XML::Element& config):
    Source(module, config)
  {
    send_tag = config["send"];
    receive_tag = config["receive"];
  }

  // Configure
  void configure(const File::Directory&,
                 const XML::Element& config) override
  {
    value = config.get_attr_int("value");
  }

  // Topo calc - pretend to be a sender or receiver
  void calculate_topology(Element::Topology& topo) override
  {
    if (!send_tag.empty())
      topo.router_senders[send_tag].push_back(this);
    if (!receive_tag.empty())
      topo.router_receivers[receive_tag].push_back(this);
  }

  void pre_tick(const TickData&) override
  {
    if (tick_order) *tick_order += id;
  }

  // Generate some data
  void tick(const TickData& td) override
  {
    send(new TestData((int)td.t));
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
// Test Subgraph
class TestSubgraph: public Source
{
  unique_ptr<Graph> subgraph;

 public:
  string *tick_order{nullptr};

  // Construct
  TestSubgraph(const Module *module, const XML::Element& config):
    Source(module, config)
  {
  }

  // Configure
  void configure(const File::Directory& base_dir,
                 const XML::Element& config) override
  {
    subgraph.reset(new Graph(graph->get_engine()));
    subgraph->configure(base_dir, config);
  }

  void calculate_topology(Element::Topology& topo) override
  {
    subgraph->calculate_topology(topo, this);
  }

  void attach(const string&, Dataflow::Acceptor *acceptor) override
  {
    subgraph->attach(acceptor);
  }

  void pre_tick(const TickData&) override
  {
    if (tick_order) *tick_order += id;
  }

  void tick(const TickData& td) override
  {
    subgraph->tick(td);
  }

  JSON::Value get_json(const string& path) const override
  {
    JSON::Value value = Element::get_json(path);
    value.set("elements", subgraph->get_json(path));
    return value;
  }

  // Shutdown
  void shutdown() override
  {
    subgraph->shutdown();
  }
};

Module TestSubgraphModule
{
  "test-subgraph", "", "", "test",
  {},
  {}, // no inputs
  { "any" }
};

Registry::NewFactory<TestSubgraph> TestSubgraphFactory;

//==========================================================================
// Test Filter
class TestFilter: public Filter
{
public:
  int value;
  string *tick_order{nullptr};

  // Construct
  TestFilter(const Module *module, const XML::Element& config):
    Filter(module, config)
  {
  }

  // Configure
  void configure(const File::Directory&,
                 const XML::Element& config) override
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

  void pre_tick(const TickData&) override
  {
    if (tick_order) *tick_order += id;
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
  MT::Mutex mutex;
  int received_data = 0;  // Accumulates
  bool pre_tick_called{false};
  bool post_tick_called{false};
  string *tick_order{nullptr};

  // Construct
  TestSink(const Module *module, const XML::Element& config):
    Sink(module, config)
  {
  }

  // Process some data
  void accept(DataPtr data) override
  {
    MT::Lock lock{mutex};
    auto td = data.check<TestData>();
    received_data += td->n;
  }

  // Notify of tick
  void pre_tick(const TickData&) override
  {
    MT::Lock lock{mutex};
    pre_tick_called = true;
    if (tick_order) *tick_order += id;
  }
  void post_tick(const TickData&) override
  {
    MT::Lock lock{mutex};
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
  engine.add_default_section("test");
  engine.element_registry.add(TestSourceModule, TestSourceFactory);
  engine.element_registry.add(TestSubgraphModule, TestSubgraphFactory);
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
    graph.configure(File::Directory("."), config.get_root());
    graph.calculate_topology();
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
    graph.configure(File::Directory("."), config.get_root());
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
    graph.configure(File::Directory("."), config.get_root());

    Element::Topology topo;
    graph.calculate_topology(topo);
  }
  catch (runtime_error e)
  {
    FAIL() << "Can't create multigraph: " << e.what() << endl;
    return;
  }
}
