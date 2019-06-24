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
  GraphTester tester{loader};

  auto lfo = tester.add("lfo").set("wave", "square")
                              .set("period", 100);
  auto smo = tester.add("smooth").set("time", 0);

  lfo.connect(smo);
  smo.connect_test();

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(1.0, v.d);
}

TEST(SmoothTest, TestSingleRCPeriodIs63Percent)
{
  GraphTester tester{loader};

  auto lfo = tester.add("lfo").set("wave", "square")
                              .set("period", 100000);
  auto smo = tester.add("smooth").set("time", 10000);

  lfo.connect(smo);
  smo.connect_test();

  tester.test(10000);

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.632, v.d, 0.001);
}

TEST(SmoothTest, TestTwoRCPeriodsIs86Percent)
{
  GraphTester tester{loader};

  auto lfo = tester.add("lfo").set("wave", "square")
                              .set("period", 100000);
  auto smo = tester.add("smooth").set("time", 10000);

  lfo.connect(smo);
  smo.connect_test();

  tester.test(20000);

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.865, v.d, 0.001);
}

TEST(SmoothTest, TestThreeRCPeriodsIs95Percent)
{
  GraphTester tester{loader};

  auto lfo = tester.add("lfo").set("wave", "square")
                              .set("period", 100000);
  auto smo = tester.add("smooth").set("time", 10000);

  lfo.connect(smo);
  smo.connect_test();

  tester.test(30000);

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.95, v.d, 0.001);
}

TEST(SmoothTest, TestFourRCPeriodsIs98Percent)
{
  GraphTester tester{loader};

  auto lfo = tester.add("lfo").set("wave", "square")
                              .set("period", 100000);
  auto smo = tester.add("smooth").set("time", 10000);

  lfo.connect(smo);
  smo.connect_test();

  tester.test(40000);

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.982, v.d, 0.001);
}

TEST(SmoothTest, TestFiveRCPeriodsIs99Percent)
{
  GraphTester tester{loader};

  auto lfo = tester.add("lfo").set("wave", "square")
                              .set("period", 100000);
  auto smo = tester.add("smooth").set("time", 10000);

  lfo.connect(smo);
  smo.connect_test();

  tester.test(50000);

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
