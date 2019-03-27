//==========================================================================
// ViGraph dataflow module: controls/wait/test-wait.cc
//
// Tests for <wait> control
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(WaitTest, TestZeroDelayDoesNotTriggerSameTick)
{
  ControlTester tester(loader, Value::Type::trigger);
  tester.test("<trigger/>",
              "<wait/>", 1);
  EXPECT_FALSE(tester.target->got("trigger"));
}

TEST(WaitTest, TestZeroDelayTriggersNextTick)
{
  ControlTester tester(loader, Value::Type::trigger);
  tester.test("<trigger/>",
              "<wait/>", 2);
  EXPECT_TRUE(tester.target->got("trigger"));
}

TEST(WaitTest, TestWaitDelayDoesntTriggerEarly)
{
  ControlTester tester(loader, Value::Type::trigger);
  tester.test("<trigger/>",
              "<wait for='2' />", 2);
  EXPECT_FALSE(tester.target->got("trigger"));
}

TEST(WaitTest, TestWaitDelayTriggers)
{
  ControlTester tester(loader, Value::Type::trigger);
  tester.test("<trigger/>",
              "<wait for='2'/>", 3);
  EXPECT_TRUE(tester.target->got("trigger"));
}

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  loader.load("../trigger/vg-module-core-control-trigger.so");
  loader.load("./vg-module-core-control-wait.so");
  return RUN_ALL_TESTS();
}
