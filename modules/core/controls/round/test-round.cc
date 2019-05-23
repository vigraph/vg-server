//==========================================================================
// ViGraph dataflow module: core/controls/round/test-round.cc
//
// Tests for <round> control
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(RoundTest, TestZeroNDoesNothing)
{
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", 1);
  auto rnd = tester.add("round").set("n", 0);

  set.connect("value", rnd, "value");
  rnd.connect_test("value", "value");

  tester.test();

  ASSERT_FALSE(tester.target->got("value"));
}

TEST(RoundTest, TestZeroDDoesNothing)
{
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", 1);
  auto rnd = tester.add("round").set("d", 0);

  set.connect("value", rnd, "value");
  rnd.connect_test("value", "value");

  tester.test();

  ASSERT_FALSE(tester.target->got("value"));
}

TEST(RoundTest, TestNearestThree)
{
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", 1.5);
  auto rnd = tester.add("round").set("n", 3);

  set.connect("value", rnd, "value");
  rnd.connect_test("value", "value");

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(3.0, v.d);
}

TEST(RoundTest, TestNearestThird)
{
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", 0.4);
  auto rnd = tester.add("round").set("d", 3);

  set.connect("value", rnd, "value");
  rnd.connect_test("value", "value");

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(1.0/3.0, v.d, 0.0001);
}

TEST(RoundTest, TestNearestTwoThird)
{
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", 0.8);
  auto rnd = tester.add("round").set("n", 2).set("d", 3);

  set.connect("value", rnd, "value");
  rnd.connect_test("value", "value");

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(2.0/3.0, v.d, 0.0001);
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
  loader.load("./vg-module-core-control-round.so");
  return RUN_ALL_TESTS();
}
