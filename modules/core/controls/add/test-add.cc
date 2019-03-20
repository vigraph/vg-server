//==========================================================================
// ViGraph dataflow module: core/controls/add/test-add.cc
//
// Tests for <add> control
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(AddTest, TestAddZeroDoesNothing)
{
  ControlTester tester(loader);
  tester.test("<set property='value' value='0.2'/>",
              "<add property='x'/>");
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(0.2, sp.v.d, 1e-5);
}

TEST(AddTest, TestAddValue)
{
  ControlTester tester(loader);
  tester.test("<set property='value' value='0.2'/>",
              "<add property='x' offset='0.13'/>");
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(0.33, sp.v.d, 1e-5);
}

TEST(AddTest, TestModifyAddOffset)
{
  ControlTester tester(loader);
  tester.test("<set target='add' property='value' value='0.2'/>",
              "<set target='add' property='offset' value='0.3'/>",
              "<add id='add' property='x'/>");
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(0.5, sp.v.d, 1e-5);
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
  loader.load("./vg-module-core-control-add.so");
  return RUN_ALL_TESTS();
}
