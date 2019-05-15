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
  tester.test("<set value='1' property='t'/>",
              "<interpolate property='foo'/>");
  ASSERT_FALSE(tester.target->got("foo"));
}

TEST(InterpolateTest, TestInterpolateWithOnePointHasEffect)
{
  ControlTester tester(loader);
  tester.test("<set value='1' property='t'/>",
              "<interpolate property='foo'>"
                "<point value='1'/>"
              "</interpolate>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& v = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(1, v.d);
}

TEST(InterpolateTest, TestInterpolateWithTwoPoints)
{
  ControlTester tester(loader);
  tester.test("<set value='0.5' property='t'/>",
              "<interpolate property='foo'>"
                "<point value='1'/>"
                "<point value='2'/>"
              "</interpolate>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& v = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(1.5, v.d);
}

TEST(InterpolateTest, TestInterpolateWithThreePointsOnBoundary)
{
  ControlTester tester(loader);
  tester.test("<set value='0.5' property='t'/>",
              "<interpolate property='foo'>"
                "<point value='1'/>"
                "<point value='2'/>"
                "<point value='3'/>"
              "</interpolate>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& v = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(2, v.d);
}

TEST(InterpolateTest, TestInterpolateWithThreePoints)
{
  ControlTester tester(loader);
  tester.test("<set value='0.75' property='t'/>",
              "<interpolate property='foo'>"
                "<point value='1'/>"
                "<point value='2'/>"
                "<point value='3'/>"
              "</interpolate>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& v = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_DOUBLE_EQ(2.5, v.d);
}

TEST(InterpolateTest, TestInterpolateWithThreePointsAtEnd)
{
  ControlTester tester(loader);
  tester.test("<set value='1' property='t'/>",
              "<interpolate property='foo'>"
                "<point value='1'/>"
                "<point value='2'/>"
                "<point value='3'/>"
              "</interpolate>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& v = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(3, v.d);
}

TEST(InterpolateTest, TestInterpolateWithSpecifiedAtTwoPoints)
{
  ControlTester tester(loader);
  tester.test("<set value='0.2' property='t'/>",
              "<interpolate property='foo'>"
                "<point value='1'/>"
                "<point t='0.4' value='2'/>"
              "</interpolate>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& v = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(1.5, v.d);
}

TEST(InterpolateTest, TestInterpolateWithSpecifiedAtTwoPointsAfterLast)
{
  ControlTester tester(loader);
  tester.test("<set value='0.5' property='t'/>",
              "<interpolate property='foo'>"
                "<point value='1'/>"
                "<point t='0.4' value='2'/>"
              "</interpolate>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& v = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(2, v.d);
}

TEST(InterpolateTest, TestInterpolateWithSpecifiedAtTwoPointsLargeAtValues)
{
  ControlTester tester(loader);
  tester.test("<set value='500' property='t'/>",
              "<interpolate property='foo'>"
                "<point value='1'/>"
                "<point t='1000' value='2'/>"
              "</interpolate>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& v = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(1.5, v.d);
}

TEST(InterpolateTest, TestInterpolateWithSpecifiedAtThreePoints)
{
  ControlTester tester(loader);
  tester.test("<set value='2.25' property='t'/>",
              "<interpolate property='foo'>"
                "<point t='1.0' value='1'/>"
                "<point t='2.0' value='2'/>"
                "<point t='3.0' value='3'/>"
              "</interpolate>");
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
