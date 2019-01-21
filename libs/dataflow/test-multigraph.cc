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

TEST(MultiGraphTest, TestGraphConstructionWithExplicitAndImplicitIDs)
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

TEST(MultiGraphTest, TestTickAll)
{
  const string& xml = R"(
    <container>
      <graph id='one'>
        <test-source/>
        <test-sink/>
      </graph>
      <graph>
        <test-source/>
        <test-filter value='2'/>
        <test-sink/>
      </graph>
    </container>
  )";

  MultiGraph mg(engine);
  construct_multigraph(xml, mg);

  // Tick it
  mg.tick_all({1, 0, Time::Duration{1}});

  Graph *g = mg.get_subgraph("one");
  ASSERT_FALSE(!g) << "Explicit subgraph IDs broken";

  Element *el = g->get_element("test-sink");
  ASSERT_FALSE(!el);
  TestSink *sink = dynamic_cast<TestSink *>(el);
  ASSERT_FALSE(!sink);
  EXPECT_EQ(1, sink->received_data);

  g = mg.get_subgraph("graph-1");
  ASSERT_FALSE(!g) << "Implicit subgraph IDs broken";

  el = g->get_element("test-sink");
  ASSERT_FALSE(!el);
  sink = dynamic_cast<TestSink *>(el);
  ASSERT_FALSE(!sink);
  EXPECT_EQ(2, sink->received_data);
}

TEST(MultiGraphTest, TestAttachAll)
{
  const string& xml = R"(
    <container>
      <graph id='one'>
        <test-source/>
      </graph>
      <graph>
        <test-source/>
        <test-filter value='2'/>
      </graph>
    </container>
  )";

  struct Catcher: public Acceptor
  {
    int received_data{0};

    void accept(DataPtr data) override
    {
      auto td = data.check<TestData>();
      received_data += td->n;
    }
  };

  unique_ptr<Catcher> catcher{new Catcher};

  MultiGraph mg(engine);
  construct_multigraph(xml, mg);

  // Attach catcher to end of all subgraphs
  mg.attach_to_all(catcher.get());

  // Tick all subgraphs
  mg.tick_all({1, 0, Time::Duration{1}});

  // Catch should have got 1+2
  ASSERT_EQ(3, catcher->received_data);
}

TEST(MultiGraphTest, TestShutdown)
{
  const string& xml = R"(
    <container>
      <graph>
        <test-source/>
      </graph>
      <graph>
        <test-source/>
      </graph>
    </container>
  )";

  MultiGraph mg(engine);
  construct_multigraph(xml, mg);

  Graph *g = mg.get_subgraph("graph-1");
  ASSERT_FALSE(!g);
  Element *el = g->get_element("test-source");
  ASSERT_FALSE(!el);
  TestSource *source1 = dynamic_cast<TestSource *>(el);
  ASSERT_FALSE(!source1);

  g = mg.get_subgraph("graph-2");
  ASSERT_FALSE(!g);
  el = g->get_element("test-source");
  ASSERT_FALSE(!el);
  TestSource *source2 = dynamic_cast<TestSource *>(el);
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
