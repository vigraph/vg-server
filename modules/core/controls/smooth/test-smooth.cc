//==========================================================================
// ViGraph dataflow module: core/controls/smooth/test-smooth.cc
//
// Tests for <smooth> control
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(SmoothTest, TestZeroRCDoesNothing)
{
  ControlTester tester(loader);
  tester.test("<lfo property='x' wave='square' phase='0.5' period='100'/>",
              "<smooth time='0' property='x'/>", 1);
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_EQ(1.0, sp.v.d);
}

TEST(SmoothTest, TestSingleRCPeriodIs63Percent)
{
  ControlTester tester(loader);
  tester.test("<lfo property='x' wave='square' phase='0.5' period='100000'/>",
              "<smooth time='10000' property='x'/>", 10000);
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(0.632, sp.v.d, 0.001);
}

TEST(SmoothTest, TestTwoRCPeriodsIs86Percent)
{
  ControlTester tester(loader);
  tester.test("<lfo property='x' wave='square' phase='0.5' period='100000'/>",
              "<smooth time='10000' property='x'/>", 20000);
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(0.865, sp.v.d, 0.001);
}

TEST(SmoothTest, TestThreeRCPeriodsIs95Percent)
{
  ControlTester tester(loader);
  tester.test("<lfo property='x' wave='square' phase='0.5' period='100000'/>",
              "<smooth time='10000' property='x'/>", 30000);
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(0.95, sp.v.d, 0.001);
}

TEST(SmoothTest, TestFourRCPeriodsIs98Percent)
{
  ControlTester tester(loader);
  tester.test("<lfo property='x' wave='square' phase='0.5' period='100000'/>",
              "<smooth time='10000' property='x'/>", 40000);
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(0.982, sp.v.d, 0.001);
}

TEST(SmoothTest, TestFiveRCPeriodsIs99Percent)
{
  ControlTester tester(loader);
  tester.test("<lfo property='x' wave='square' phase='0.5' period='100000'/>",
              "<smooth time='10000' property='x'/>", 50000);
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(0.993, sp.v.d, 0.001);
}

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  loader.load("../lfo/vg-module-core-control-lfo.so");
  loader.load("./vg-module-core-control-smooth.so");
  return RUN_ALL_TESTS();
}
