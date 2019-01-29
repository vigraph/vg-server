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
  ControlTester tester(loader);
  tester.test("<set value='1'/>",
              "<interpolate property='foo'/>");
  ASSERT_FALSE(tester.target->got("foo"));
}

TEST(InterpolateTest, TestInterpolateWithOnePointHasEffect)
{
  ControlTester tester(loader);
  tester.test("<set value='1'/>",
              "<interpolate>"
                "<point foo='1'/>"
              "</interpolate>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& sp = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_EQ(1, sp.v.d);
}

TEST(InterpolateTest, TestInterpolateWithTwoPoints)
{
  ControlTester tester(loader);
  tester.test("<set value='0.5'/>",
              "<interpolate>"
                "<point foo='1'/>"
                "<point foo='2'/>"
              "</interpolate>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& sp = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_EQ(1.5, sp.v.d);
}

TEST(InterpolateTest, TestInterpolateWithThreePointsOnBoundary)
{
  ControlTester tester(loader);
  tester.test("<set value='0.5'/>",
              "<interpolate>"
                "<point foo='1'/>"
                "<point foo='2'/>"
                "<point foo='3'/>"
              "</interpolate>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& sp = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_EQ(2, sp.v.d);
}

TEST(InterpolateTest, TestInterpolateWithThreePoints)
{
  ControlTester tester(loader);
  tester.test("<set value='0.75'/>",
              "<interpolate>"
                "<point foo='1'/>"
                "<point foo='2'/>"
                "<point foo='3'/>"
              "</interpolate>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& sp = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_DOUBLE_EQ(2.5, sp.v.d);
}

TEST(InterpolateTest, TestInterpolateWithThreePointsAtEnd)
{
  ControlTester tester(loader);
  tester.test("<set value='1'/>",
              "<interpolate>"
                "<point foo='1'/>"
                "<point foo='2'/>"
                "<point foo='3'/>"
              "</interpolate>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& sp = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_EQ(3, sp.v.d);
}

TEST(InterpolateTest, TestInterpolateWithMissingValueUsesLast)
{
  ControlTester tester(loader);
  tester.test("<set value='0.75'/>",
              "<interpolate>"
                "<point foo='1'/>"
                "<point foo='2'/>"
                "<point/>"
              "</interpolate>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& sp = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_EQ(2, sp.v.d);
}

TEST(InterpolateTest, TestInterpolateWithSpecifiedAtTwoPoints)
{
  ControlTester tester(loader);
  tester.test("<set value='0.2'/>",
              "<interpolate>"
                "<point foo='1'/>"
                "<point at='0.4' foo='2'/>"
              "</interpolate>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& sp = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_EQ(1.5, sp.v.d);
}

TEST(InterpolateTest, TestInterpolateWithSpecifiedAtTwoPointsAfterLast)
{
  ControlTester tester(loader);
  tester.test("<set value='0.5'/>",
              "<interpolate>"
                "<point foo='1'/>"
                "<point at='0.4' foo='2'/>"
              "</interpolate>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& sp = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_EQ(2, sp.v.d);
}

TEST(InterpolateTest, TestInterpolateWithSpecifiedAtTwoPointsLargeAtValues)
{
  ControlTester tester(loader);
  tester.test("<set value='500'/>",
              "<interpolate>"
                "<point foo='1'/>"
                "<point at='1000' foo='2'/>"
              "</interpolate>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& sp = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_EQ(1.5, sp.v.d);
}

TEST(InterpolateTest, TestInterpolateWithSpecifiedAtThreePoints)
{
  ControlTester tester(loader);
  tester.test("<set value='2.25'/>",
              "<interpolate>"
                "<point at='1.0' foo='1'/>"
                "<point at='2.0' foo='2'/>"
                "<point at='3.0' foo='3'/>"
              "</interpolate>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& sp = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_EQ(2.25, sp.v.d);
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
