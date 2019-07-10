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

TEST_F(GraphTest, TestGraphTickAndFiltering)
{
  TestGraph graph(engine);
  auto source = graph.add("test-source");
  auto filter1 = graph.add("test-filter").set("value", 2);
  auto filter2 = graph.add("test-filter").set("value", 3);
  auto sinke = graph.add("test-sink", "SINK");

  source.connect("default", filter1, "default");
  filter1.connect("default", filter2, "default");
  filter2.connect("default", sinke, "default");
  graph.setup();

  auto sink = graph.get<TestSink>("SINK");
  ASSERT_NE(nullptr, sink);

  EXPECT_FALSE(sink->pre_tick_called);
  ASSERT_NO_THROW(graph.pre_tick({1.0, 0, Time::Duration{1}, 1}));
  EXPECT_TRUE(sink->pre_tick_called);

  ASSERT_NO_THROW(graph.tick({1.0, 0, Time::Duration{1}, 1}));

  EXPECT_FALSE(sink->post_tick_called);
  ASSERT_NO_THROW(graph.post_tick({1.0, 0, Time::Duration{1}, 1}));
  EXPECT_TRUE(sink->post_tick_called);

  EXPECT_EQ(6, sink->received_data);
  ASSERT_NO_THROW(graph.tick({2.0, 1, Time::Duration{1}, 1}));
  EXPECT_EQ(18, sink->received_data);
}

TEST_F(GraphTest, TestGraphTickAndMultipleSources)
{
  TestGraph graph(engine);
  auto source1 = graph.add("test-source");
  auto source2 = graph.add("test-source");
  auto filter = graph.add("test-filter").set("value", 2);
  auto sinke = graph.add("test-sink", "SINK");

  source1.connect("default", filter, "default");
  source2.connect("default", filter, "default");
  filter.connect("default", sinke, "default");
  graph.setup();

  auto sink = graph.get<TestSink>("SINK");
  ASSERT_NE(nullptr, sink);

  ASSERT_NO_THROW(graph.tick({1.0, 0, Time::Duration{1}, 1}));
  EXPECT_EQ(4, sink->received_data);
  ASSERT_NO_THROW(graph.tick({2.0, 1, Time::Duration{1}, 1}));
  EXPECT_EQ(12, sink->received_data);
}

TEST_F(GraphTest, TestGraphSimpleTickOrdering)
{
  TestGraph graph(engine);
  auto filter_e = graph.add("test-filter", "f");
  auto source_e = graph.add("test-source", "S");
  auto sink_e = graph.add("test-sink", "s");

  source_e.connect("default", filter_e, "default");
  filter_e.connect("default", sink_e, "default");
  graph.setup();

  string tick_order;

  auto source = graph.get<TestSource>("S");
  ASSERT_NE(nullptr, source);
  source->tick_order = &tick_order;

  auto filter = graph.get<TestFilter>("f");
  ASSERT_NE(nullptr, filter);
  filter->tick_order = &tick_order;

  auto sink = graph.get<TestSink>("s");
  ASSERT_NE(nullptr, sink);
  sink->tick_order = &tick_order;

  ASSERT_NO_THROW(graph.pre_tick({1.0, 0, Time::Duration{1}, 1}));
  EXPECT_EQ("Sfs", tick_order);
}

TEST_F(GraphTest, TestGraphTickOrderingWithoutRouting)
{
  TestGraph graph(engine);
  graph.add("test-source", "S1");
  graph.add("test-source", "S2");
  graph.setup();

  string tick_order;

  auto source1 = graph.get<TestSource>("S1");
  ASSERT_NE(nullptr, source1);
  source1->tick_order = &tick_order;

  auto source2 = graph.get<TestSource>("S2");
  ASSERT_NE(nullptr, source1);
  source2->tick_order = &tick_order;

  ASSERT_NO_THROW(graph.pre_tick({1.0, 0, Time::Duration{1}, 1}));
  EXPECT_EQ("S1S2", tick_order);
}

TEST_F(GraphTest, TestGraphTickOrderingWithRouting)
{
  TestGraph graph(engine);
  graph.add("test-source", "S1").set("receive", "foo");
  graph.add("test-source", "S2").set("send", "foo");
  graph.setup();

  string tick_order;

  auto source1 = graph.get<TestSource>("S1");
  ASSERT_NE(nullptr, source1);
  source1->tick_order = &tick_order;

  auto source2 = graph.get<TestSource>("S2");
  ASSERT_NE(nullptr, source1);
  source2->tick_order = &tick_order;

  ASSERT_NO_THROW(graph.pre_tick({1.0, 0, Time::Duration{1}, 1}));
  EXPECT_EQ("S2S1", tick_order);
}

TEST_F(GraphTest, TestGraphTickOrderingWithSubgraphSender)
{
  TestGraph graph(engine);
  graph.add("test-source", "S1").set("receive", "foo");
  graph.add("test-subgraph", "G");

  TestGraph *subgraph = new TestGraph(engine);
  subgraph->add("test-source", "S2").set("send", "foo");
  auto sg = graph.get<TestSubgraph>("G");
  ASSERT_FALSE(!sg);
  sg->set_subgraph(subgraph);

  graph.setup();

  string tick_order;
  sg->tick_order = &tick_order;

  auto source1 = graph.get<TestSource>("S1");
  ASSERT_NE(nullptr, source1);
  source1->tick_order = &tick_order;

  ASSERT_NO_THROW(graph.pre_tick({1.0, 0, Time::Duration{1}, 1}));
  EXPECT_EQ("GS1", tick_order);
}

TEST_F(GraphTest, TestGraphTickOrderingWithSubgraphReceiver)
{
  TestGraph graph(engine);
  graph.add("test-subgraph", "G");
  graph.add("test-source", "S1").set("send", "foo");

  TestGraph *subgraph = new TestGraph(engine);
  subgraph->add("test-source", "S2").set("receive", "foo");
  auto sg = graph.get<TestSubgraph>("G");
  ASSERT_FALSE(!sg);
  sg->set_subgraph(subgraph);

  graph.setup();

  string tick_order;
  sg->tick_order = &tick_order;

  auto source1 = graph.get<TestSource>("S1");
  ASSERT_NE(nullptr, source1);
  source1->tick_order = &tick_order;

  ASSERT_NO_THROW(graph.pre_tick({1.0, 0, Time::Duration{1}, 1}));
  EXPECT_EQ("S1G", tick_order);
}

TEST_F(GraphTest, TestGraphTickOrderingWithPeerSubgraphSenderAndReceiver)
{
  TestGraph graph(engine);
  graph.add("test-subgraph", "G1");
  graph.add("test-subgraph", "G2");
  string tick_order;

  TestGraph *subgraph1 = new TestGraph(engine);
  subgraph1->add("test-source").set("receive", "foo");
  auto sg1 = graph.get<TestSubgraph>("G1");
  ASSERT_FALSE(!sg1);
  sg1->set_subgraph(subgraph1);
  sg1->tick_order = &tick_order;

  TestGraph *subgraph2 = new TestGraph(engine);
  subgraph2->add("test-source").set("send", "foo");
  auto sg2 = graph.get<TestSubgraph>("G2");
  ASSERT_FALSE(!sg2);
  sg2->set_subgraph(subgraph2);
  sg2->tick_order = &tick_order;

  graph.setup();

  ASSERT_NO_THROW(graph.pre_tick({1.0, 0, Time::Duration{1}, 1}));
  EXPECT_EQ("G2G1", tick_order);
}

TEST_F(GraphTest, TestSubgraphExternalConnection)
{
  TestGraph graph(engine);
  auto graph_e = graph.add("test-subgraph", "G");
  auto sink_e = graph.add("test-sink", "SINK");

  TestGraph *subgraph = new TestGraph(engine);
  subgraph->add("test-source");
  subgraph->setup();

  auto sg = graph.get<TestSubgraph>("G");
  ASSERT_FALSE(!sg);
  sg->set_subgraph(subgraph);

  graph_e.connect("default", sink_e, "default");
  graph.setup();

  auto sink = graph.get<TestSink>("SINK");
  ASSERT_NE(nullptr, sink);

  ASSERT_NO_THROW(graph.tick({1.0, 0, Time::Duration{1}, 1}));
  EXPECT_EQ(1, sink->received_data);
}

TEST_F(GraphTest, TestSubgraphExternalConnectionIsRestoredAfterDeleteOfTail)
{
  TestGraph graph(engine);
  auto graph_e = graph.add("test-subgraph", "G");
  auto sink_e = graph.add("test-sink", "SINK");

  TestGraph *subgraph = new TestGraph(engine);
  auto source_e = subgraph->add("test-source");
  auto filter_e = subgraph->add("test-filter", "F").set("value", 2);
  source_e.connect("default", filter_e, "default");
  subgraph->setup();

  auto sg = graph.get<TestSubgraph>("G");
  ASSERT_FALSE(!sg);
  sg->set_subgraph(subgraph);

  graph_e.connect("default", sink_e, "default");
  graph.setup();

  // Delete the filter
  subgraph->delete_item("F");

  // Run without it - output should come direct from source as in previous
  // test
  auto sink = graph.get<TestSink>("SINK");
  ASSERT_NE(nullptr, sink);

  EXPECT_EQ(0, sink->received_data);
  ASSERT_NO_THROW(graph.tick({1.0, 0, Time::Duration{1}, 1}));
  EXPECT_EQ(1, sink->received_data);
}

TEST_F(GraphTest, TestGraphConstructionFromAndOutputToJSON)
{
  const string& json = R"(
    [
      {
        "id": "s1",
        "outputs":
        {
          "default":
          [
            {
              "element": "f1",
              "prop": "default"
            }
          ]
        },
        "props": {},
        "type": "test:test-source"
      },
      {
        "id": "f1",
        "outputs":
        {
          "default":
          [
            {
              "element": "f2",
              "prop": "default"
            }
          ]
        },
        "props": {},
        "type": "test:test-filter"
      },
      {
        "id": "f2",
        "props": {},
        "type": "test:test-filter"
      }
    ]
  )";

  Graph graph(engine);
  construct_graph_json(json, graph);

  JSON::Value value = graph.get_json();
  ASSERT_EQ(JSON::Value::Type::ARRAY, value.type);
  ASSERT_EQ(3, value.a.size());
  // Note ordered by dependency
  EXPECT_EQ("s1", value.a[0]["id"].as_str());
  EXPECT_EQ("f1", value.a[1]["id"].as_str());
  EXPECT_EQ("f2", value.a[2]["id"].as_str());
}

TEST_F(GraphTest, TestGraphShutdown)
{
  TestGraph graph(engine);
  graph.add("test-source", "S");
  graph.setup();

  auto source = graph.get<TestSource>("S");
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
