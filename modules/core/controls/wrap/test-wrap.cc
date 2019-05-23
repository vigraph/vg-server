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
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", 0.2);
  auto wrp = tester.add("wrap");

  set.connect("value", wrp, "value");
  wrp.connect_test("value", "value");

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.2, v.d, 1e-5);
}

TEST(WrapTest, TestDefaultWrapAbsoluteValueUpper)
{
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", 1.3);
  auto wrp = tester.add("wrap");

  set.connect("value", wrp, "value");
  wrp.connect_test("value", "value");

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.3, v.d, 1e-5);
}

TEST(WrapTest, TestDefaultWrapAbsoluteValueLower)
{
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", -0.3);
  auto wrp = tester.add("wrap");

  set.connect("value", wrp, "value");
  wrp.connect_test("value", "value");

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.7, v.d, 1e-5);
}

TEST(WrapTest, TestSpecifiedWrapAbsoluteValueUpper)
{
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", 4.3);
  auto wrp = tester.add("wrap").set("min", 3).set("max", 4);

  set.connect("value", wrp, "value");
  wrp.connect_test("value", "value");

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(3.3, v.d, 1e-5);
}

TEST(WrapTest, TestSpecifiedWrapAbsoluteValueLower)
{
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", 2.7);
  auto wrp = tester.add("wrap").set("min", 3).set("max", 4);

  set.connect("value", wrp, "value");
  wrp.connect_test("value", "value");

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(3.7, v.d, 1e-5);
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
