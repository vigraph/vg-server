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
  GraphTester tester{loader, Value::Type::trigger};

  auto trg = tester.add("trigger");
  auto wat = tester.add("wait");

  trg.connect(wat, "trigger");
  wat.connect_test("trigger");

  tester.test();

  EXPECT_FALSE(tester.target->got("trigger"));
}

TEST(WaitTest, TestZeroDelayTriggersNextTick)
{
  GraphTester tester{loader, Value::Type::trigger};

  auto trg = tester.add("trigger");
  auto wat = tester.add("wait");

  trg.connect(wat, "trigger");
  wat.connect_test("trigger");

  tester.test(2);

  EXPECT_TRUE(tester.target->got("trigger"));
}

TEST(WaitTest, TestWaitDelayDoesntTriggerEarly)
{
  GraphTester tester{loader, Value::Type::trigger};

  auto trg = tester.add("trigger");
  auto wat = tester.add("wait").set("for", 2);

  trg.connect(wat, "trigger");
  wat.connect_test("trigger");

  tester.test(2);

  EXPECT_FALSE(tester.target->got("trigger"));
}

TEST(WaitTest, TestWaitDelayTriggers)
{
  GraphTester tester{loader, Value::Type::trigger};

  auto trg = tester.add("trigger");
  auto wat = tester.add("wait").set("for", 2);

  trg.connect(wat, "trigger");
  wat.connect_test("trigger");

  tester.test(5);

  EXPECT_TRUE(tester.target->got("trigger"));

  // Only called once
  EXPECT_EQ(1, tester.target->sets_called);
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
