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
  tester.test("<lfo wave='square' phase='0.5' period='100'/>",
              "<smooth time='0' />", 1);
  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(1.0, v.d);
}

TEST(SmoothTest, TestSingleRCPeriodIs63Percent)
{
  ControlTester tester(loader);
  tester.test("<lfo wave='square' phase='0.5' period='100000'/>",
              "<smooth time='10000'/>", 10000);
  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.632, v.d, 0.001);
}

TEST(SmoothTest, TestTwoRCPeriodsIs86Percent)
{
  ControlTester tester(loader);
  tester.test("<lfo wave='square' phase='0.5' period='100000'/>",
              "<smooth time='10000'/>", 20000);
  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.865, v.d, 0.001);
}

TEST(SmoothTest, TestThreeRCPeriodsIs95Percent)
{
  ControlTester tester(loader);
  tester.test("<lfo wave='square' phase='0.5' period='100000'/>",
              "<smooth time='10000'/>", 30000);
  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.95, v.d, 0.001);
}

TEST(SmoothTest, TestFourRCPeriodsIs98Percent)
{
  ControlTester tester(loader);
  tester.test("<lfo wave='square' phase='0.5' period='100000'/>",
              "<smooth time='10000'/>", 40000);
  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.982, v.d, 0.001);
}

TEST(SmoothTest, TestFiveRCPeriodsIs99Percent)
{
  ControlTester tester(loader);
  tester.test("<lfo wave='square' phase='0.5' period='100000'/>",
              "<smooth time='10000'/>", 50000);
  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.993, v.d, 0.001);
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
