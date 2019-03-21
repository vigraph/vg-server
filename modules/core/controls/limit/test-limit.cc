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
  ControlTester tester(loader);
  tester.test("<set property='value' value='0.2'/>",
              "<limit property='x'/>");
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(0.2, sp.v.d, 1e-5);
}

TEST(LimitTest, TestDefaultLimitAbsoluteValueUpper)
{
  ControlTester tester(loader);
  tester.test("<set property='value' value='1.5'/>",
              "<limit property='x'/>");
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(1.0, sp.v.d, 1e-5);
}

TEST(LimitTest, TestDefaultLimitAbsoluteValueLower)
{
  ControlTester tester(loader);
  tester.test("<set property='value' value='-0.5'/>",
              "<limit property='x'/>");
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(0.0, sp.v.d, 1e-5);
}

TEST(LimitTest, TestSpecifiedLimitAbsoluteValueUpper)
{
  ControlTester tester(loader);
  tester.test("<set property='value' value='5.5'/>",
              "<limit property='x' min='3' max='4'/>");
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(4.0, sp.v.d, 1e-5);
}

TEST(LimitTest, TestSpecifiedLimitAbsoluteValueLower)
{
  ControlTester tester(loader);
  tester.test("<set property='value' value='-0.5'/>",
              "<limit property='x' min='3' max='4'/>");
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(3.0, sp.v.d, 1e-5);
}

TEST(LimitTest, TestLimitDeltaValue)
{
  ControlTester tester(loader);
  tester.test("<modify property='value' delta='1.5'/>",
              "<limit property='x'/>");
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  // Limit converts to absolute set
  EXPECT_FALSE(sp.increment);
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(1.0, sp.v.d, 1e-5);
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
  loader.load("../modify/vg-module-core-control-modify.so");
  loader.load("./vg-module-core-control-limit.so");
  return RUN_ALL_TESTS();
}
