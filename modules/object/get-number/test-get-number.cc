//==========================================================================
// ViGraph dataflow module: object/get-number/test-get-number.cc
//
// Tests for object get number filter
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#include "../object-module.h"
#include "../../module-test.h"
#include <cmath>

class GetNumberTest: public GraphTester
{
public:
  GetNumberTest()
  {
    loader.load("./vg-module-object-get-number.so");
  }
};

const auto sample_rate = 1;

TEST_F(GetNumberTest, TestExtractAValidNumber)
{
  auto& getn = add("object/get-number")
    .set("property", string("foo"));

  auto datas = vector<Data>(1);
  auto& data = datas[0];
  data.json = JSON::Value(JSON::Value::OBJECT);
  data.json.set("foo", 42);

  auto& ds = add_source(datas);
  ds.connect("output", getn, "input");

  auto ns = vector<Number>{};
  auto& sink = add_sink(ns, sample_rate);
  getn.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, ns.size());
  ASSERT_EQ(42, ns[0]);
}

TEST_F(GetNumberTest, TestExtractInvalidNumber)
{
  auto& getn = add("object/get-number")
    .set("property", string("bar"));

  auto datas = vector<Data>(1);
  auto& data = datas[0];
  data.json = JSON::Value(JSON::Value::OBJECT);
  data.json.set("foo", 42);

  auto& ds = add_source(datas);
  ds.connect("output", getn, "input");

  auto ns = vector<Number>{};
  auto& sink = add_sink(ns, sample_rate);
  getn.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, ns.size());
  ASSERT_EQ(0, ns[0]);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
