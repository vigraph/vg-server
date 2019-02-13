//==========================================================================
// ViGraph dataflow module: core/controls/toggle/test-toggle.cc
//
// Tests for <toggle> control
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(ToggleTest, TestOneTrigger)
{
  ControlTester tester(loader, Value::Type::any);
  tester.test("<set property='trigger'/>",
              "<toggle />");
  ASSERT_TRUE(tester.target->got("trigger"));
}

TEST(ToggleTest, TestTwoTriggers)
{
  ControlTester tester(loader, Value::Type::any);
  tester.test("<set target='tog' property='trigger'/>",
              "<set property='trigger'/>",
              "<toggle id='tog' />");
  ASSERT_TRUE(tester.target->got("clear"));
}

TEST(ToggleTest, TestOneOn)
{
  ControlTester tester(loader, Value::Type::any);
  tester.test("<set property='on' value='0.25'/>",
              "<toggle />");
  ASSERT_TRUE(tester.target->got("on"));
  const auto& sp = tester.target->get("on");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_EQ(0.25, sp.v.d);
}

TEST(ToggleTest, TestTwoOns)
{
  ControlTester tester(loader, Value::Type::any);
  tester.test("<set target='tog' property='on' value='0.25'/>",
              "<set property='on' value='0.25'/>",
              "<toggle id='tog' />");
  ASSERT_TRUE(tester.target->got("off"));
  const auto& sp = tester.target->get("off");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_EQ(0.25, sp.v.d);
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
  loader.load("./vg-module-core-control-toggle.so");
  return RUN_ALL_TESTS();
}
