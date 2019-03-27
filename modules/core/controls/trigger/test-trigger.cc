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
  ControlTester tester(loader, Value::Type::trigger);
  tester.test("<trigger/>");
  ASSERT_TRUE(tester.target->got("trigger"));
  const auto& sp = tester.target->get("trigger");
  EXPECT_EQ(Value::Type::trigger, sp.v.type);
}

TEST(TriggerTest, TestTriggerWithWaitNotTriggeredHasNoEffect)
{
  ControlTester tester(loader, Value::Type::trigger);
  tester.test("<trigger wait='yes'/>");
  EXPECT_FALSE(tester.target->got("trigger"));
}

TEST(TriggerTest, TestTriggerWithAutoWaitNotTriggeredHasNoEffect)
{
  ControlTester tester(loader, Value::Type::trigger);
  tester.test("<trigger wait='yes'/>",
              "<trigger/>", 2);
  EXPECT_FALSE(tester.target->got("trigger"));
}

TEST(TriggerTest, TestTriggerWithWaitTriggeredHasEffect)
{
  ControlTester tester(loader, Value::Type::trigger);
  tester.test("<trigger/>",
              "<trigger wait='yes'/>", 2);
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
