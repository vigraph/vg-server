//==========================================================================
// ViGraph dataflow module: core/controls/multiply/test-multiply.cc
//
// Tests for <multiply> control
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(MultiplyTest, TestMultiplyDefaultDoesNothing)
{
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", 0.2);
  auto mul = tester.add("multiply");

  set.connect("value", mul, "value");
  mul.connect_test("value", "value");

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.2, v.d, 1e-5);
}

TEST(MultiplyTest, TestMultiplyBy2)
{
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", 1.5);
  auto mul = tester.add("multiply").set("factor", 2);

  set.connect("value", mul, "value");
  mul.connect_test("value", "value");

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(3.0, v.d, 1e-5);
}

TEST(MultiplyTest, TestMultiplyBySetFactor)
{
  GraphTester tester{loader};

  auto setv = tester.add("set").set("value", 1.5);
  auto setf = tester.add("set").set("value", 2);
  auto mul = tester.add("multiply").set("factor", 2);

  setv.connect("value", mul, "value");
  setf.connect("value", mul, "factor");
  mul.connect_test("value", "value");

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(3.0, v.d, 1e-5);
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
  loader.load("./vg-module-core-control-multiply.so");
  return RUN_ALL_TESTS();
}
