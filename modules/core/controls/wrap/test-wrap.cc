//==========================================================================
// ViGraph dataflow module: controls/wrap/test-wrap.cc
//
// Tests for <wrap> control
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(WrapTest, TestWrapDoesNothingInRange)
{
  ControlTester tester(loader);
  tester.test("<set value='0.2'/>",
              "<wrap/>");
  ASSERT_TRUE(tester.target->got("value"));
  const auto& sp = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(0.2, sp.v.d, 1e-5);
}

TEST(WrapTest, TestDefaultWrapAbsoluteValueUpper)
{
  ControlTester tester(loader);
  tester.test("<set value='1.3'/>",
              "<wrap/>");
  ASSERT_TRUE(tester.target->got("value"));
  const auto& sp = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(0.3, sp.v.d, 1e-5);
}

TEST(WrapTest, TestDefaultWrapAbsoluteValueLower)
{
  ControlTester tester(loader);
  tester.test("<set value='-0.3'/>",
              "<wrap/>");
  ASSERT_TRUE(tester.target->got("value"));
  const auto& sp = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(0.7, sp.v.d, 1e-5);
}

TEST(WrapTest, TestSpecifiedWrapAbsoluteValueUpper)
{
  ControlTester tester(loader);
  tester.test("<set value='4.3'/>",
              "<wrap min='3' max='4'/>");
  ASSERT_TRUE(tester.target->got("value"));
  const auto& sp = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(3.3, sp.v.d, 1e-5);
}

TEST(WrapTest, TestSpecifiedWrapAbsoluteValueLower)
{
  ControlTester tester(loader);
  tester.test("<set value='2.7'/>",
              "<wrap min='3' max='4'/>");
  ASSERT_TRUE(tester.target->got("value"));
  const auto& sp = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(3.7, sp.v.d, 1e-5);
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
  loader.load("./vg-module-core-control-wrap.so");
  return RUN_ALL_TESTS();
}
