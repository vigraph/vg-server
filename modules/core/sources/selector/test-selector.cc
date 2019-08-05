//==========================================================================
// ViGraph dataflow module: core/sources/selector/test-selector.cc
//
// Tests for <selector> source
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(SelectorTest, TestGetEmptyFromJSON)
{
  GraphTester tester(loader);

  auto selector = tester.add("selector");
  tester.test();

  auto json = selector->get_json();
  ASSERT_EQ("selector1", json["id"].as_str());
  ASSERT_EQ("core:selector", json["type"].as_str());

  auto& graphs = json["graphs"];
  ASSERT_EQ(JSON::Value::ARRAY, graphs.type);
  EXPECT_TRUE(graphs.a.empty());

  auto& props = json["props"];
  ASSERT_EQ(JSON::Value::OBJECT, props.type);

  ASSERT_EQ(JSON::Value::FALSE_, props["retrigger"].type);
  ASSERT_EQ(-1, props["value"].as_int());
}

TEST(SelectorTest, TestSetFromJSON)
{
  GraphTester tester(loader);

  auto selector = tester.add("selector");
  tester.test();

  // Create with one subgraph
  JSON::Value sjson(JSON::Value::OBJECT);
  auto& sgraphs = sjson.put("graphs", JSON::Value::ARRAY);

  JSON::Value graph1(JSON::Value::OBJECT);
  graph1.put("id", "graph1");
  graph1.put("elements", JSON::Value::ARRAY);
  sgraphs.a.push_back(graph1);

  selector->set_json("", sjson);

  // Read back
  auto json = selector->get_json();
  ASSERT_EQ("selector1", json["id"].as_str());
  ASSERT_EQ("core:selector", json["type"].as_str());

  auto& graphs = json["graphs"];
  ASSERT_EQ(JSON::Value::ARRAY, graphs.type);
  EXPECT_EQ(1, graphs.a.size());
  auto& sg = graphs.a[0];
  EXPECT_EQ("graph1", sg["id"].as_str());
  EXPECT_EQ(JSON::Value::ARRAY, sg["elements"].type);
}

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  loader.load("./vg-module-core-source-selector.so");
  return RUN_ALL_TESTS();
}
