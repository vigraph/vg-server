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
  auto& source = graph.add("test-source");
  auto& filter1 = graph.add("test-filter").set("value", 2.0);
  auto& filter2 = graph.add("test-filter").set("value", 3.0);
  auto& sinke = graph.add("test-sink", "SINK");

  source.connect("output", filter1, "input");
  filter1.connect("output", filter2, "input");
  filter2.connect("output", sinke, "input");
  graph.setup();

  auto sink = graph.get<TestSink>("SINK");
  ASSERT_NE(nullptr, sink);

  ASSERT_NO_THROW(engine.tick(Time::Duration{1}));
  EXPECT_EQ(6, sink->received_data);
  ASSERT_NO_THROW(engine.tick(Time::Duration{2}));
  EXPECT_EQ(18, sink->received_data);
}

TEST_F(GraphTest, TestGraphTickAndMultipleSources)
{
  TestGraph graph(engine);
  auto& source1 = graph.add("test-source");
  auto& source2 = graph.add("test-source");
  auto& filter = graph.add("test-filter").set("value", 2.0);
  auto& sinke = graph.add("test-sink", "SINK");

  source1.connect("output", filter, "input");
  source2.connect("output", filter, "input");
  filter.connect("output", sinke, "input");
  graph.setup();

  auto sink = graph.get<TestSink>("SINK");
  ASSERT_NE(nullptr, sink);

  ASSERT_NO_THROW(engine.tick(Time::Duration{1}));
  EXPECT_EQ(4, sink->received_data);
  ASSERT_NO_THROW(engine.tick(Time::Duration{2}));
  EXPECT_EQ(12, sink->received_data);
}

TEST_F(GraphTest, TestGraphSimpleTickOrdering)
{
  TestGraph graph(engine);
  auto& filter_e = graph.add("test-filter", "f");
  auto& source_e = graph.add("test-source", "S");
  auto& sink_e = graph.add("test-sink", "s");

  source_e.connect("output", filter_e, "input");
  filter_e.connect("output", sink_e, "input");
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

  ASSERT_NO_THROW(engine.tick(Time::Duration{1}));
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

  ASSERT_NO_THROW(engine.tick(Time::Duration{1}));
  EXPECT_EQ("S1S2", tick_order);
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
