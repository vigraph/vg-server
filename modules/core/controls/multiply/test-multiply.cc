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
  ControlTester tester(loader);
  tester.test("<set property='value' value='0.2'/>",
              "<multiply property='x'/>");
  ASSERT_TRUE(tester.target->got("x"));
  const auto& v = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.2, v.d, 1e-5);
}

TEST(MultiplyTest, TestMultiplyBy2)
{
  ControlTester tester(loader);
  tester.test("<set property='value' value='1.5'/>",
              "<multiply factor='2' property='x'/>");
  ASSERT_TRUE(tester.target->got("x"));
  const auto& v = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(3.0, v.d, 1e-5);
}

TEST(MultiplyTest, TestMultiplyBySetFactor)
{
  ControlTester tester(loader);
  tester.test("<set target='m' property='factor' value='2'/>",
              "<set property='value' value='1.5'/>",
              "<multiply id='m' property='x'/>");
  ASSERT_TRUE(tester.target->got("x"));
  const auto& v = tester.target->get("x");
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
