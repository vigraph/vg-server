//==========================================================================
// ViGraph dataflow module: core/sources/clone/test-clone.cc
//
// Tests for <clone> source
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(CloneTest, TestGetEmptyFromJSON)
{
  GraphTester tester(loader);

  auto clone = tester.add("clone")
    .set("n", 10);
  tester.test();

  auto json = clone->get_json();
  ASSERT_EQ("clone1", json["id"].as_str());
  ASSERT_EQ("core:clone", json["type"].as_str());

  auto& props = json["props"];
  ASSERT_EQ(JSON::Value::OBJECT, props.type);

  ASSERT_EQ(10, props["n"].as_int());
}

TEST(CloneTest, TestSetEmptySubgraphFromJSON)
{
  GraphTester tester(loader);

  auto clone = tester.add("clone")
    .set("n", 10);
  tester.test();

  // Create with subgraph
  JSON::Value sjson(JSON::Value::OBJECT);
  sjson.put("graph", JSON::Value::ARRAY);
  clone->set_json("", sjson);

  // Read back
  auto json = clone->get_json();
  ASSERT_EQ("clone1", json["id"].as_str());
  ASSERT_EQ("core:clone", json["type"].as_str());

  auto& graph = json["graph"];
  ASSERT_EQ(JSON::Value::ARRAY, graph.type);
  EXPECT_TRUE(graph.a.empty());
}



int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  loader.load("./vg-module-core-source-clone.so");
  return RUN_ALL_TESTS();
}
