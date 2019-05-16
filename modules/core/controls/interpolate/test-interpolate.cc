//==========================================================================
// ViGraph dataflow module: controls/interpolate/test-interpolate.cc
//
// Tests for <interpolate> control
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(InterpolateTest, TestInterpolateWithNoPointsHasNoEffect)
{
  GraphTester tester(loader);

  auto set = tester.add("set").set("value", 1);
  auto itp = tester.add("interpolate");

  set.connect("value", itp, "t");
  itp.connect_test("value", "foo");

  tester.test();
  ASSERT_FALSE(tester.target->got("foo"));
}

TEST(InterpolateTest, TestInterpolateWithOnePointHasEffect)
{
  GraphTester tester(loader);

  auto set = tester.add("set").set("value", 1);

  JSON::Value json(JSON::Value::ARRAY);
  json.add(JSON::Value(JSON::Value::OBJECT)).set("value", 1);
  auto itp = tester.add("interpolate").set("curve", json);

  set.connect("value", itp, "t");
  itp.connect_test("value", "foo");

  tester.test();

  ASSERT_TRUE(tester.target->got("foo"));
  const auto& v = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(1, v.d);
}

TEST(InterpolateTest, TestInterpolateWithTwoPoints)
{
  GraphTester tester(loader);

  auto set = tester.add("set").set("value", 0.5);

  JSON::Value json(JSON::Value::ARRAY);
  json.add(JSON::Value(JSON::Value::OBJECT)).set("value", 1);
  json.add(JSON::Value(JSON::Value::OBJECT)).set("value", 2);
  auto itp = tester.add("interpolate").set("curve", json);

  set.connect("value", itp, "t");
  itp.connect_test("value", "foo");

  tester.test();

  ASSERT_TRUE(tester.target->got("foo"));
  const auto& v = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(1.5, v.d);
}

TEST(InterpolateTest, TestInterpolateWithThreePointsOnBoundary)
{
  GraphTester tester(loader);

  auto set = tester.add("set").set("value", 0.5);

  JSON::Value json(JSON::Value::ARRAY);
  json.add(JSON::Value(JSON::Value::OBJECT)).set("value", 1);
  json.add(JSON::Value(JSON::Value::OBJECT)).set("value", 2);
  json.add(JSON::Value(JSON::Value::OBJECT)).set("value", 3);
  auto itp = tester.add("interpolate").set("curve", json);

  set.connect("value", itp, "t");
  itp.connect_test("value", "foo");

  tester.test();

  ASSERT_TRUE(tester.target->got("foo"));
  const auto& v = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(2, v.d);
}

TEST(InterpolateTest, TestInterpolateWithThreePoints)
{
  GraphTester tester(loader);

  auto set = tester.add("set").set("value", 0.75);

  JSON::Value json(JSON::Value::ARRAY);
  json.add(JSON::Value(JSON::Value::OBJECT)).set("value", 1);
  json.add(JSON::Value(JSON::Value::OBJECT)).set("value", 2);
  json.add(JSON::Value(JSON::Value::OBJECT)).set("value", 3);
  auto itp = tester.add("interpolate").set("curve", json);

  set.connect("value", itp, "t");
  itp.connect_test("value", "foo");

  tester.test();

  ASSERT_TRUE(tester.target->got("foo"));
  const auto& v = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_DOUBLE_EQ(2.5, v.d);
}

TEST(InterpolateTest, TestInterpolateWithThreePointsAtEnd)
{
  GraphTester tester(loader);

  auto set = tester.add("set").set("value", 1);

  JSON::Value json(JSON::Value::ARRAY);
  json.add(JSON::Value(JSON::Value::OBJECT)).set("value", 1);
  json.add(JSON::Value(JSON::Value::OBJECT)).set("value", 2);
  json.add(JSON::Value(JSON::Value::OBJECT)).set("value", 3);
  auto itp = tester.add("interpolate").set("curve", json);

  set.connect("value", itp, "t");
  itp.connect_test("value", "foo");

  tester.test();

  ASSERT_TRUE(tester.target->got("foo"));
  const auto& v = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(3, v.d);
}

TEST(InterpolateTest, TestInterpolateWithSpecifiedTAtTwoPoints)
{
  GraphTester tester(loader);

  auto set = tester.add("set").set("value", 0.2);

  JSON::Value json(JSON::Value::ARRAY);
  json.add(JSON::Value(JSON::Value::OBJECT)).set("value", 1);
  json.add(JSON::Value(JSON::Value::OBJECT)).set("t", 0.4).set("value", 2);
  auto itp = tester.add("interpolate").set("curve", json);

  set.connect("value", itp, "t");
  itp.connect_test("value", "foo");

  tester.test();

  ASSERT_TRUE(tester.target->got("foo"));
  const auto& v = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(1.5, v.d);
}

TEST(InterpolateTest, TestInterpolateWithSpecifiedAtTwoPointsAfterLast)
{
  GraphTester tester(loader);

  auto set = tester.add("set").set("value", 0.5);

  JSON::Value json(JSON::Value::ARRAY);
  json.add(JSON::Value(JSON::Value::OBJECT)).set("value", 1);
  json.add(JSON::Value(JSON::Value::OBJECT)).set("t", 0.4).set("value", 2);
  auto itp = tester.add("interpolate").set("curve", json);

  set.connect("value", itp, "t");
  itp.connect_test("value", "foo");

  tester.test();

  ASSERT_TRUE(tester.target->got("foo"));
  const auto& v = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(2, v.d);
}

TEST(InterpolateTest, TestInterpolateWithSpecifiedAtTwoPointsLargeAtValues)
{
  GraphTester tester(loader);

  auto set = tester.add("set").set("value", 500);

  JSON::Value json(JSON::Value::ARRAY);
  json.add(JSON::Value(JSON::Value::OBJECT)).set("value", 1);
  json.add(JSON::Value(JSON::Value::OBJECT)).set("t", 1000).set("value", 2);
  auto itp = tester.add("interpolate").set("curve", json);

  set.connect("value", itp, "t");
  itp.connect_test("value", "foo");

  tester.test();

  ASSERT_TRUE(tester.target->got("foo"));
  const auto& v = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(1.5, v.d);
}

TEST(InterpolateTest, TestInterpolateWithSpecifiedAtThreePoints)
{
  GraphTester tester(loader);

  auto set = tester.add("set").set("value", 2.25);

  JSON::Value json(JSON::Value::ARRAY);
  json.add(JSON::Value(JSON::Value::OBJECT)).set("t", 1.0).set("value", 1);
  json.add(JSON::Value(JSON::Value::OBJECT)).set("t", 2.0).set("value", 2);
  json.add(JSON::Value(JSON::Value::OBJECT)).set("t", 3.0).set("value", 3);
  auto itp = tester.add("interpolate").set("curve", json);

  set.connect("value", itp, "t");
  itp.connect_test("value", "foo");

  tester.test();

  ASSERT_TRUE(tester.target->got("foo"));
  const auto& v = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(2.25, v.d);
}

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  loader.load("../set/vg-module-core-control-set.so");
  loader.load("./vg-module-core-control-interpolate.so");
  return RUN_ALL_TESTS();
}
