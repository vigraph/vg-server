//==========================================================================
// ViGraph dataflow machine: test-multigraph.cc
//
// Tests for multigraph container structure
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-dataflow.h"
#include <gtest/gtest.h>
#include "ot-log.h"

namespace {

#include "test-elements.h"

// This goes when XML does !!!
TEST_F(GraphTest, TestGraphConstructionWithExplicitAndImplicitIDs)
{
  const string& xml = R"(
    <container>
      <graph id='one'>
        <test-source value='42'/>
      </graph>
      <graph>
        <test-source value='555'/>
      </graph>
    </container>
  )";

  MultiGraph mg(engine);
  construct_multigraph(xml, mg);

  Graph *g = mg.get_subgraph("one");
  ASSERT_FALSE(!g) << "Explicit subgraph IDs broken";

  // Implicitly created ID
  Element *el = g->get_element("test-source");
  ASSERT_FALSE(!el);
  TestSource *source = dynamic_cast<TestSource *>(el);
  ASSERT_FALSE(!source);
  EXPECT_EQ(42, source->value);

  g = mg.get_subgraph("graph-1");
  ASSERT_FALSE(!g) << "Implicit subgraph IDs broken";

  // Same implicitly created ID, different object
  el = g->get_element("test-source");
  ASSERT_FALSE(!el);
  source = dynamic_cast<TestSource *>(el);
  ASSERT_FALSE(!source);
  EXPECT_EQ(555, source->value);
}

TEST_F(GraphTest, TestTickAll)
{
  auto graph1 = new TestGraph(engine);
  auto source1 = graph1->add("test-source");
  auto sink1 = graph1->add("test-sink", "SINK1");
  source1.connect("default", sink1, "default");
  graph1->setup();

  auto graph2 = new TestGraph(engine);
  auto source2 = graph2->add("test-source");
  auto filter2 = graph2->add("test-filter").set("value", 2);
  auto sink2 = graph2->add("test-sink", "SINK2");
  source2.connect("default", filter2, "default");
  filter2.connect("default", sink2, "default");
  graph2->setup();

  MultiGraph mg(engine);
  mg.add_subgraph("G1", graph1);
  mg.add_subgraph("G2", graph2);
  ASSERT_EQ(graph1, mg.get_subgraph("G1"));
  ASSERT_EQ(graph2, mg.get_subgraph("G2"));

  // Tick it
  mg.tick_all({1, 0, Time::Duration{1}, 1});

  auto s1 = graph1->get<TestSink>("SINK1");
  ASSERT_FALSE(!s1);
  EXPECT_EQ(1, s1->received_data);

  auto s2 = graph2->get<TestSink>("SINK2");
  ASSERT_FALSE(!s2);
  EXPECT_EQ(2, s2->received_data);
}

TEST_F(GraphTest, TestDataSendUp)
{
  auto graph1 = new TestGraph(engine);
  graph1->add("test-source");
  graph1->setup();

  auto graph2 = new TestGraph(engine);
  auto source2 = graph2->add("test-source");
  auto filter2 = graph2->add("test-filter").set("value", 2);
  source2.connect("default", filter2, "default");
  graph2->setup();

  MultiGraph mg(engine);
  mg.add_subgraph("G1", graph1);
  mg.add_subgraph("G2", graph2);
  ASSERT_EQ(graph1, mg.get_subgraph("G1"));
  ASSERT_EQ(graph2, mg.get_subgraph("G2"));

  struct Catcher: public Acceptor
  {
    int received_data{0};

    void accept(DataPtr data) override
    {
      auto td = data.check<TestData>();
      received_data += td->n;
    }
  };

  shared_ptr<Catcher> catcher{new Catcher};

  // Attach catcher to end of all subgraphs
  mg.set_send_up_function([catcher](DataPtr data)
                          { catcher->accept(data); });

  // Tick all subgraphs
  mg.tick_all({1, 0, Time::Duration{1}, 1});

  // Catch should have got 1+2
  ASSERT_EQ(3, catcher->received_data);
}

TEST_F(GraphTest, TestShutdown)
{
  auto graph1 = new TestGraph(engine);
  graph1->add("test-source", "S");
  graph1->setup();

  auto graph2 = new TestGraph(engine);
  graph2->add("test-source", "S");
  graph2->setup();

  MultiGraph mg(engine);
  mg.add_subgraph("G1", graph1);
  mg.add_subgraph("G2", graph2);
  ASSERT_EQ(graph1, mg.get_subgraph("G1"));
  ASSERT_EQ(graph2, mg.get_subgraph("G2"));

  TestSource *source1 = graph1->get<TestSource>("S");
  ASSERT_FALSE(!source1);

  TestSource *source2 = graph2->get<TestSource>("S");
  ASSERT_FALSE(!source2);

  ASSERT_FALSE(source1->shutdown_called);
  ASSERT_FALSE(source2->shutdown_called);
  mg.shutdown();
  ASSERT_TRUE(source1->shutdown_called);
  ASSERT_TRUE(source2->shutdown_called);
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
