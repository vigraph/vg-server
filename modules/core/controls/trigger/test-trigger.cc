//==========================================================================
// ViGraph dataflow module: controls/trigger/test-trigger.cc
//
// Tests for <trigger> control
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(TriggerTest, TestTriggerTriggers)
{
  GraphTester tester{loader, Value::Type::trigger};

  auto trg = tester.add("trigger");

  trg.connect_test("trigger");

  tester.test();

  ASSERT_TRUE(tester.target->got("trigger"));
  const auto& v = tester.target->get("trigger");
  EXPECT_EQ(Value::Type::trigger, v.type);
}

TEST(TriggerTest, TestTriggerWithWaitNotTriggeredHasNoEffect)
{
  GraphTester tester{loader, Value::Type::trigger};

  auto trg = tester.add("trigger").set("wait", true);

  trg.connect_test("trigger");

  tester.test();

  EXPECT_FALSE(tester.target->got("trigger"));
}

TEST(TriggerTest, TestTriggerWithAutoWaitNotTriggeredHasNoEffect)
{
  GraphTester tester{loader, Value::Type::trigger};

  auto trg = tester.add("trigger").set("wait", true);

  trg.connect_test("trigger");

  tester.test(2);

  EXPECT_FALSE(tester.target->got("trigger"));
}

TEST(TriggerTest, TestTriggerWithWaitTriggeredHasEffect)
{
  GraphTester tester{loader, Value::Type::trigger};

  auto trg = tester.add("trigger");
  auto trg2 = tester.add("trigger").set("wait", true);

  trg.connect(trg2, "trigger");
  trg2.connect_test("trigger");

  tester.test(2);

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
  loader.load("./vg-module-core-control-trigger.so");
  return RUN_ALL_TESTS();
}
