//==========================================================================
// ViGraph dataflow machine: test-graph.cc
//
// Tests for basic dataflow graph structure
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"
#include <gtest/gtest.h>
#include "ot-log.h"

namespace {

#include "test-elements.h"

TEST(GraphTest, TestSourceConstructionWithExplicitID)
{
  const string& xml = R"(
    <graph>
      <test-source id='t1' value='42'/>
    </graph>
  )";

  Graph graph(engine);
  construct_graph(xml, graph);

  Element *el = graph.get_element("t1");
  ASSERT_FALSE(!el);
  TestSource *source = dynamic_cast<TestSource *>(el);
  ASSERT_FALSE(!source);
  EXPECT_EQ(42, source->value);
}

TEST(GraphTest, TestFilterConstructionAndAutoID)
{
  const string& xml = R"(
    <graph>
      <test-source/>
      <test-filter value='33'/>
      <test-filter value='44'/>
    </graph>
  )";

  Graph graph(engine);
  construct_graph(xml, graph);

  Element *el = graph.get_element("test-filter");
  ASSERT_FALSE(!el);
  TestFilter *f = dynamic_cast<TestFilter *>(el);
  ASSERT_FALSE(!f);
  EXPECT_EQ(33, f->value);

  el = graph.get_element("test-filter-1");
  ASSERT_FALSE(!el);
  f = dynamic_cast<TestFilter *>(el);
  ASSERT_FALSE(!f);
  EXPECT_EQ(44, f->value);
}

TEST(GraphTest, TestMultiSourceFilterConstruction)
{
  const string& xml = R"(
    <graph>
      <test-source/>
      <test-source/>
      <test-filter/>
    </graph>
  )";

  Graph graph(engine);
  construct_graph(xml, graph);
}

TEST(GraphTest, TestExplicitAcceptorConstruction)
{
  const string& xml = R"(
    <graph>
      <test-source acceptor="filter"/>
      <test-filter id="filter"/>
    </graph>
  )";

  Graph graph(engine);
  construct_graph(xml, graph);
}

TEST(GraphTest, TestExplicitReversedAcceptorConstruction)
{
  const string& xml = R"(
    <graph>
      <test-filter id="filter"/>
      <test-source acceptor="filter"/>
    </graph>
  )";

  Graph graph(engine);
  construct_graph(xml, graph);
}

TEST(GraphTest, TestMultipleSubelementAcceptorConstruction)
{
  const string& xml = R"(
    <graph>
      <test-source>
        <acceptor id="filter1"/>
        <acceptor id="filter2"/>
      </test-source>
      <test-filter id="filter1"/>
      <test-filter id="filter2"/>
    </graph>
  )";

  Graph graph(engine);
  construct_graph(xml, graph);
}

TEST(GraphTest, TestExplicitAcceptorConstructionFailsWhenNoUnboundInput)
{
  const string& xml = R"(
    <graph>
      <test-source acceptor="filter"/>
      <test-filter id="orphan"/>
      <test-filter id="filter"/>
    </graph>
  )";

  Graph graph(engine);
  construct_graph_should_fail(xml, graph);
}

TEST(GraphTest, TestExplicitAcceptorConstructionFailsWhenAcceptorNotThere)
{
  const string& xml = R"(
    <graph>
      <test-source acceptor="foo"/>
    </graph>
  )";

  Graph graph(engine);
  construct_graph_should_fail(xml, graph);
}

TEST(GraphTest, TestMultipleSubelementAcceptorConstructionFailsWhenNotThere)
{
  const string& xml = R"(
    <graph>
      <test-source>
        <acceptor id="filter1"/>
        <acceptor id="filterx"/>
      </test-source>
      <test-filter id="filter1"/>
      <test-filter id="filter2"/>
    </graph>
  )";

  Graph graph(engine);
  construct_graph_should_fail(xml, graph);
}

TEST(GraphTest, TestGraphTickAndFiltering)
{
  const string& xml = R"(
    <graph>
      <test-source/>
      <test-filter value='2'/>
      <test-filter value='3'/>
      <test-sink/>
    </graph>
  )";

  Graph graph(engine);
  construct_graph(xml, graph);

  Element *el = graph.get_element("test-sink");
  ASSERT_FALSE(!el);
  TestSink *sink = dynamic_cast<TestSink *>(el);
  ASSERT_FALSE(!sink);

  EXPECT_FALSE(sink->pre_tick_called);
  ASSERT_NO_THROW(graph.pre_tick({1.0, 0, Time::Duration{1}}));
  EXPECT_TRUE(sink->pre_tick_called);

  ASSERT_NO_THROW(graph.tick({1.0, 0, Time::Duration{1}}));

  EXPECT_FALSE(sink->post_tick_called);
  ASSERT_NO_THROW(graph.post_tick({1.0, 0, Time::Duration{1}}));
  EXPECT_TRUE(sink->post_tick_called);

  EXPECT_EQ(6, sink->received_data);
  ASSERT_NO_THROW(graph.tick({2.0, 1, Time::Duration{1}}));
  EXPECT_EQ(18, sink->received_data);
}

TEST(GraphTest, TestGraphTickAndMultipleSources)
{
  const string& xml = R"(
    <graph>
      <test-source/>
      <test-source/>
      <test-filter value='2'/>
      <test-sink/>
    </graph>
  )";

  Graph graph(engine);
  construct_graph(xml, graph);

  Element *el = graph.get_element("test-sink");
  ASSERT_FALSE(!el);
  TestSink *sink = dynamic_cast<TestSink *>(el);
  ASSERT_FALSE(!sink);

  ASSERT_NO_THROW(graph.tick({1.0, 0, Time::Duration{1}}));
  EXPECT_EQ(4, sink->received_data);
  ASSERT_NO_THROW(graph.tick({2.0, 1, Time::Duration{1}}));
  EXPECT_EQ(12, sink->received_data);
}

TEST(GraphTest, TestGraphSimpleTickOrdering)
{
  const string& xml = R"(
    <graph>
      <test-filter id='f' acceptor='s'/>
      <test-source id='S' acceptor='f'/>
      <test-sink id='s'/>
    </graph>
  )";

  Graph graph(engine);
  construct_graph(xml, graph);

  string tick_order;

  Element *el = graph.get_element("S");
  ASSERT_FALSE(!el);
  TestSource *source = dynamic_cast<TestSource *>(el);
  ASSERT_FALSE(!source);
  source->tick_order = &tick_order;

  el = graph.get_element("f");
  ASSERT_FALSE(!el);
  TestFilter *filter = dynamic_cast<TestFilter *>(el);
  ASSERT_FALSE(!filter);
  filter->tick_order = &tick_order;

  el = graph.get_element("s");
  ASSERT_FALSE(!el);
  TestSink *sink = dynamic_cast<TestSink *>(el);
  ASSERT_FALSE(!sink);
  sink->tick_order = &tick_order;

  ASSERT_NO_THROW(graph.pre_tick({1.0, 0, Time::Duration{1}}));
  EXPECT_EQ("Sfs", tick_order);
}

TEST(GraphTest, TestGraphTickOrderingWithoutRouting)
{
  const string& xml = R"(
    <graph>
      <test-source id='S1'/>
      <test-source id='S2'/>
    </graph>
  )";

  Graph graph(engine);
  construct_graph(xml, graph);

  string tick_order;

  Element *el = graph.get_element("S1");
  ASSERT_FALSE(!el);
  TestSource *s1 = dynamic_cast<TestSource *>(el);
  ASSERT_FALSE(!s1);
  s1->tick_order = &tick_order;

  el = graph.get_element("S2");
  ASSERT_FALSE(!el);
  TestSource *s2 = dynamic_cast<TestSource *>(el);
  ASSERT_FALSE(!s2);
  s2->tick_order = &tick_order;

  ASSERT_NO_THROW(graph.pre_tick({1.0, 0, Time::Duration{1}}));
  EXPECT_EQ("S1S2", tick_order);
}

TEST(GraphTest, TestGraphTickOrderingWithRouting)
{
  const string& xml = R"(
    <graph>
      <test-source id='S1' receive='foo'/>
      <test-source id='S2' send='foo'/>
    </graph>
  )";

  Graph graph(engine);
  construct_graph(xml, graph);

  string tick_order;

  Element *el = graph.get_element("S1");
  ASSERT_FALSE(!el);
  TestSource *s1 = dynamic_cast<TestSource *>(el);
  ASSERT_FALSE(!s1);
  s1->tick_order = &tick_order;

  el = graph.get_element("S2");
  ASSERT_FALSE(!el);
  TestSource *s2 = dynamic_cast<TestSource *>(el);
  ASSERT_FALSE(!s2);
  s2->tick_order = &tick_order;

  ASSERT_NO_THROW(graph.pre_tick({1.0, 0, Time::Duration{1}}));
  EXPECT_EQ("S2S1", tick_order);
}

TEST(GraphTest, TestGraphTickOrderingWithSubgraphSender)
{
  const string& xml = R"(
    <graph>
      <test-source id='S1' receive='foo'/>
      <test-subgraph id='G'>
        <test-source id='S2' send='foo'/>
      </test-subgraph>
    </graph>
  )";

  Graph graph(engine);
  construct_graph(xml, graph);

  string tick_order;

  Element *el = graph.get_element("G");
  ASSERT_FALSE(!el);
  TestSubgraph *g = dynamic_cast<TestSubgraph *>(el);
  ASSERT_FALSE(!g);
  g->tick_order = &tick_order;

  el = graph.get_element("S1");
  ASSERT_FALSE(!el);
  TestSource *s1 = dynamic_cast<TestSource *>(el);
  ASSERT_FALSE(!s1);
  s1->tick_order = &tick_order;

  ASSERT_NO_THROW(graph.pre_tick({1.0, 0, Time::Duration{1}}));
  EXPECT_EQ("GS1", tick_order);
}

TEST(GraphTest, TestGraphTickOrderingWithSubgraphReceiver)
{
  const string& xml = R"(
    <graph>
      <test-subgraph id='G'>
        <test-source id='S2' receive='foo'/>
      </test-subgraph>
      <test-source id='S1' send='foo'/>
    </graph>
  )";

  Graph graph(engine);
  construct_graph(xml, graph);

  string tick_order;

  Element *el = graph.get_element("G");
  ASSERT_FALSE(!el);
  TestSubgraph *g = dynamic_cast<TestSubgraph *>(el);
  ASSERT_FALSE(!g);
  g->tick_order = &tick_order;

  el = graph.get_element("S1");
  ASSERT_FALSE(!el);
  TestSource *s1 = dynamic_cast<TestSource *>(el);
  ASSERT_FALSE(!s1);
  s1->tick_order = &tick_order;

  ASSERT_NO_THROW(graph.pre_tick({1.0, 0, Time::Duration{1}}));
  EXPECT_EQ("S1G", tick_order);
}

TEST(GraphTest, TestGraphTickOrderingWithPeerSubgraphSenderAndReceiver)
{
  const string& xml = R"(
    <graph>
      <test-subgraph id='G1'>
        <test-source receive='foo'/>
      </test-subgraph>
      <test-subgraph id='G2'>
        <test-source send='foo'/>
      </test-subgraph>
    </graph>
  )";

  Graph graph(engine);
  construct_graph(xml, graph);

  string tick_order;

  Element *el = graph.get_element("G1");
  ASSERT_FALSE(!el);
  TestSubgraph *g1 = dynamic_cast<TestSubgraph *>(el);
  ASSERT_FALSE(!g1);
  g1->tick_order = &tick_order;

  el = graph.get_element("G2");
  ASSERT_FALSE(!el);
  TestSubgraph *g2 = dynamic_cast<TestSubgraph *>(el);
  ASSERT_FALSE(!g2);
  g2->tick_order = &tick_order;

  ASSERT_NO_THROW(graph.pre_tick({1.0, 0, Time::Duration{1}}));
  EXPECT_EQ("G2G1", tick_order);
}

TEST(GraphTest, TestGraphGetJSON)
{
  const string& xml = R"(
    <graph>
      <test-source id='s1'/>
      <test-filter id='f1'/>
      <test-filter id='f2'/>
    </graph>
  )";

  Graph graph(engine);
  construct_graph(xml, graph);

  JSON::Value value = graph.get_json();
  ASSERT_EQ(JSON::Value::Type::ARRAY, value.type);
  ASSERT_EQ(3, value.a.size());
  // Note ordered by dependency
  EXPECT_EQ("s1", value.a[0]["id"].as_str());
  EXPECT_EQ("f1", value.a[1]["id"].as_str());
  EXPECT_EQ("f2", value.a[2]["id"].as_str());
}

TEST(GraphTest, TestGraphShutdown)
{
  const string& xml = R"(
    <graph>
      <test-source/>
    </graph>
  )";

  Graph graph(engine);
  construct_graph(xml, graph);

  Element *el = graph.get_element("test-source");
  ASSERT_FALSE(!el);
  TestSource *source = dynamic_cast<TestSource *>(el);
  ASSERT_FALSE(!source);

  ASSERT_FALSE(source->shutdown_called);
  graph.shutdown();
  ASSERT_TRUE(source->shutdown_called);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }
  Init::Sequence::run();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
