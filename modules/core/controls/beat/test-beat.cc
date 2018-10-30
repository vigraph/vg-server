//==========================================================================
// ViGraph dataflow module: core/controls/beat/test-beat.cc
//
// Tests for <beat> control
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(BeatTest, TestInterval)
{
  ControlTester tester(loader, Value::Type::trigger);
  tester.test("<beat interval='2' property='go'/>", 10);
  ASSERT_TRUE(tester.target->got("go"));
  const auto& sp = tester.target->get("go");
  ASSERT_EQ(Value::Type::trigger, sp.v.type);
  EXPECT_EQ(5, tester.target->sets_called);
}

TEST(BeatTest, TestIntervalWithTriggerDoesntAutoRun)
{
  ControlTester tester(loader, Value::Type::trigger);
  tester.test("<set property='start' type='trigger' wait='yes'/>",
              "<beat interval='2' property='go'/>", 10);
  ASSERT_FALSE(tester.target->got("go"));
}

TEST(BeatTest, TestIntervalWithTriggeredStartRuns)
{
  ControlTester tester(loader, Value::Type::trigger);
  tester.test("<set property='start' type='trigger'/>",
              "<beat interval='2' property='go'/>", 10);
  ASSERT_TRUE(tester.target->got("go"));
  const auto& sp = tester.target->get("go");
  ASSERT_EQ(Value::Type::trigger, sp.v.type);
  EXPECT_EQ(5, tester.target->sets_called);
}

TEST(BeatTest, TestIntervalWithTriggeredStartAndStopRunsAndStops)
{
  ControlTester tester(loader, Value::Type::trigger);
  // ! Note the set(start) is after the beat(stop) so that it acts
  // after the initial trigger at 0.  To avoid this needs a <delay/> which
  // only fires once
  tester.test({
      "<beat target='b' interval='5' property='stop'/>",
      "<set property='start' type='trigger'/>",
      "<beat id='b' interval='2.0' property='go'/>"
        }, 10);
  ASSERT_TRUE(tester.target->got("go"));
  const auto& sp = tester.target->get("go");
  ASSERT_EQ(Value::Type::trigger, sp.v.type);
  EXPECT_EQ(3, tester.target->sets_called);
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
  loader.load("./vg-module-core-control-beat.so");
  return RUN_ALL_TESTS();
}
