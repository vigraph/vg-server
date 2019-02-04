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
  ControlTester tester(loader);
  tester.test("<set property='x' value='1'/>",
              "<round n='0' property='x'/>", 1);
  ASSERT_FALSE(tester.target->got("x"));
}

TEST(RoundTest, TestZeroDDoesNothing)
{
  ControlTester tester(loader);
  tester.test("<set property='x' value='1'/>",
              "<round d='0' property='x'/>", 1);
  ASSERT_FALSE(tester.target->got("x"));
}

TEST(RoundTest, TestNearestThree)
{
  ControlTester tester(loader);
  tester.test("<set property='x' value='1.5'/>",
              "<round n='3' property='x'/>", 1);
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_EQ(3.0, sp.v.d);
}

TEST(RoundTest, TestNearestThird)
{
  ControlTester tester(loader);
  tester.test("<set property='x' value='0.4'/>",
              "<round d='3' property='x'/>", 1);
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(1.0/3.0, sp.v.d, 0.0001);
}

TEST(RoundTest, TestNearestTwoThird)
{
  ControlTester tester(loader);
  tester.test("<set property='x' value='0.8'/>",
              "<round n='2' d='3' property='x'/>", 1);
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(2.0/3.0, sp.v.d, 0.0001);
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
