//==========================================================================
// ViGraph dataflow module: core/controls/limit/test-limit.cc
//
// Tests for <limit> control
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(LimitTest, TestLimitDoesNothingInRange)
{
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", 0.2);
  auto limit = tester.add("limit");

  set.connect("value", limit, "value");
  limit.connect_test("value", "value");

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.2, v.d, 1e-5);
}

TEST(LimitTest, TestDefaultLimitAbsoluteValueUpper)
{
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", 1.5);
  auto limit = tester.add("limit");

  set.connect("value", limit, "value");
  limit.connect_test("value", "value");

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(1.0, v.d, 1e-5);
}

TEST(LimitTest, TestDefaultLimitAbsoluteValueLower)
{
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", -0.5);
  auto limit = tester.add("limit");

  set.connect("value", limit, "value");
  limit.connect_test("value", "value");

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.0, v.d, 1e-5);
}

TEST(LimitTest, TestSpecifiedLimitAbsoluteValueUpper)
{
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", 5.5);
  auto limit = tester.add("limit").set("min", 3).set("max", 4);

  set.connect("value", limit, "value");
  limit.connect_test("value", "value");

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(4.0, v.d, 1e-5);
}

TEST(LimitTest, TestSpecifiedLimitAbsoluteValueLower)
{
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", -0.5);
  auto limit = tester.add("limit").set("min", 3).set("max", 4);

  set.connect("value", limit, "value");
  limit.connect_test("value", "value");

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(3.0, v.d, 1e-5);
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
  loader.load("./vg-module-core-control-limit.so");
  return RUN_ALL_TESTS();
}
