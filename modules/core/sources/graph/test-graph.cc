//==========================================================================
// ViGraph dataflow module: core/sources/graph/test-graph.cc
//
// Tests for <graph> source
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(GraphTest, TestGetEmptyFromJSON)
{
  GraphTester tester(loader);

  auto graph = tester.add("graph");
  tester.test();

  auto json = graph->get_json();
  ASSERT_EQ("graph1", json["id"].as_str());
  ASSERT_EQ("core:graph", json["type"].as_str());

  auto& props = json["props"];
  ASSERT_EQ(JSON::Value::OBJECT, props.type);
}

TEST(GraphTest, TestSetEmptySubgraphFromJSON)
{
  GraphTester tester(loader);

  auto graph = tester.add("graph");
  tester.test();

  // Create with subgraph
  JSON::Value sjson(JSON::Value::OBJECT);
  sjson.put("elements", JSON::Value::ARRAY);
  graph->set_json("", sjson);

  // Read back
  auto json = graph->get_json();
  ASSERT_EQ("graph1", json["id"].as_str());
  ASSERT_EQ("core:graph", json["type"].as_str());

  auto& elements = json["elements"];
  ASSERT_EQ(JSON::Value::ARRAY, elements.type);
  EXPECT_TRUE(elements.a.empty());
}

TEST(GraphTest, TestGetSubgraphFromJSON)
{
  GraphTester tester(loader);

  auto graph = tester.add("graph");
  tester.test();

  // Create with subgraph
  JSON::Value sjson(JSON::Value::OBJECT);
  sjson.put("elements", JSON::Value::ARRAY);
  graph->set_json("", sjson);

  // Read back
  auto elements = graph->get_json("elements");
  ASSERT_EQ(JSON::Value::ARRAY, elements.type);
  EXPECT_TRUE(elements.a.empty());
}

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  loader.load("./vg-module-core-source-graph.so");
  return RUN_ALL_TESTS();
}
