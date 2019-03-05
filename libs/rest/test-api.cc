//==========================================================================
// ViGraph REST API library: test-api.cc
//
// Tests for REST API
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-rest.h"
#include "ot-log.h"
#include <gtest/gtest.h>

namespace {

#include "../dataflow/test-elements.h"

using namespace ViGraph;
using namespace ViGraph::REST;
using namespace ViGraph::Dataflow;

TEST(APITest, TestDoNothing)
{
  API rest(engine);
}

TEST(APITest, TestGETWithNoPathFails)
{
  API rest(engine);
  JSON::Value data;
  API::Result result = rest.handle_request("GET", "/", data);
  ASSERT_EQ(API::Result::Code::not_found, result.code);
}

TEST(APITest, TestGETWithBogusPathFails)
{
  API rest(engine);
  JSON::Value data;
  API::Result result = rest.handle_request("GET", "/foo/bar", data);
  ASSERT_EQ(API::Result::Code::not_found, result.code);
}

TEST(APITest, TestGETWithEmptyGraphRootIsEmpty)
{
  API rest(engine);
  JSON::Value data;
  API::Result result = rest.handle_request("GET", "/graph", data);
  ASSERT_EQ(API::Result::Code::ok, result.code);
  ASSERT_EQ(JSON::Value::Type::ARRAY, result.value.type);
  EXPECT_EQ(0, result.value.a.size());
}

TEST(APITest, TestPUTOnGraphRootFails)
{
  API rest(engine);
  JSON::Value data;
  API::Result result = rest.handle_request("PUT", "/graph", data);
  ASSERT_EQ(API::Result::Code::method_not_allowed, result.code);
}

TEST(APITest, TestGETWithSingleItemGraphRoot)
{
  const string& xml = R"(
    <graph>
      <test-source id='s1'/>
    </graph>
  )";

  construct_graph(xml, engine.get_graph());

  API rest(engine);
  JSON::Value data;
  API::Result result = rest.handle_request("GET", "/graph", data);
  ASSERT_EQ(API::Result::Code::ok, result.code);
  ASSERT_EQ(JSON::Value::Type::ARRAY, result.value.type);
  EXPECT_EQ(1, result.value.a.size());
  EXPECT_EQ("s1", result.value.a[0]["id"].as_str());
}

TEST(APITest, TestGETWithThreeItemsGraphRoot)
{
  const string& xml = R"(
    <graph>
      <test-source id='s1'/>
      <test-filter/>
      <test-sink id='s2'/>
    </graph>
  )";

  construct_graph(xml, engine.get_graph());

  API rest(engine);
  JSON::Value data;
  API::Result result = rest.handle_request("GET", "/graph", data);
  ASSERT_EQ(API::Result::Code::ok, result.code);
  ASSERT_EQ(JSON::Value::Type::ARRAY, result.value.type);
  EXPECT_EQ(3, result.value.a.size());
  // Note ordered by ID
  EXPECT_EQ("s1", result.value.a[0]["id"].as_str());
  EXPECT_EQ("s2", result.value.a[1]["id"].as_str());
  EXPECT_EQ("test-filter", result.value.a[2]["id"].as_str());
}

} // anonymous namespace

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
