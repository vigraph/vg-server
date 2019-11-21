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

const auto sample_rate = 1.0;

//==========================================================================
// Test Source
class TestSource: public SimpleElement
{
private:
  TestSource *create_clone() const override
  {
    return new TestSource{module};
  }

public:
  bool shutdown_called = false;
  Output<double> output;
  string *tick_order{nullptr};

  // Construct
  using SimpleElement::SimpleElement;

  // Generate some data
  void tick(const TickData& td) override
  {
    auto buffer = output.get_buffer();
    const auto nsamples = td.samples_in_tick(sample_rate);
    for (auto i = 0u; i < nsamples; ++i)
      buffer.data.push_back(td.start + 1);
    if (tick_order) *tick_order += get_id();
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
class TestFilter: public SimpleElement
{
private:
  TestFilter *create_clone() const override
  {
    return new TestFilter{module};
  }

public:
  Setting<double> value;
  Input<double> input;
  Output<double> output;
  string *tick_order{nullptr};

  // Construct
  using SimpleElement::SimpleElement;

  // Process some data
  void tick(const TickData&) override
  {
    auto& in = input.get_buffer();
    auto out = output.get_buffer();
    for (const auto i: in)
      out.data.push_back(i * value.get());
    if (tick_order) *tick_order += get_id();
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
class TestSink: public SimpleElement
{
private:
  TestSink *create_clone() const override
  {
    return new TestSink{module};
  }

public:
  MT::Mutex mutex;
  Input<double> input;
  double received_data = 0;  // Accumulates
  string *tick_order{nullptr};

  // Construct
  using SimpleElement::SimpleElement;

  // Tick
  void tick(const TickData&) override
  {
    MT::Lock lock{mutex};
    const auto& in = input.get_buffer();
    for (const auto i: in)
      received_data += i;
    if (tick_order) *tick_order += get_id();
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
class TestGraph
{
  Engine& engine;
  int id_serial{0};

public:
  TestGraph(Engine& _engine):
    engine{_engine}
  {}

  GraphElement& add(const string& name, string id = "")
  {
    if (id.empty())
      id = name + Text::itos(++id_serial);
    GraphElement *e = engine.create(name, id);
    if (!e) throw runtime_error("Can't create element "+name);
    engine.get_graph().add(e);
    return *e;
  }

  // Get an element of a given type, nullptr if not found or wrong type
  template <typename T> T *get(const string& name)
  {
    auto *el = engine.get_graph().get_element(name);
    if (!el) return nullptr;
    return dynamic_cast<T *>(el);
  }

  void setup()
  {
    engine.set_tick_interval(Time::Duration{1.0});
    engine.setup();
    engine.update_elements();
  }

  void shutdown()
  {
    engine.get_graph().shutdown();
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
    engine.element_registry.add("test", "test-source", TestSourceFactory);
    engine.element_registry.add("test", "test-filter", TestFilterFactory);
    engine.element_registry.add("test", "test-sink", TestSinkFactory);
  }
};
