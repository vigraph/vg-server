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
  GraphTester tester(loader, Value::Type::trigger);

  tester.add("beat")
    .set("interval", 2)
    .connect_test("trigger", "go");

  tester.test(10);

  ASSERT_TRUE(tester.target->got("go"));
  const auto& v = tester.target->get("go");
  ASSERT_EQ(Value::Type::trigger, v.type);
  EXPECT_EQ(5, tester.target->sets_called);
}

TEST(BeatTest, TestBPM)
{
  GraphTester tester(loader, Value::Type::trigger);

  tester.add("beat")
    .set("bpm", 30)
    .connect_test("trigger", "go");

  tester.test(10);

  ASSERT_TRUE(tester.target->got("go"));
  const auto& v = tester.target->get("go");
  ASSERT_EQ(Value::Type::trigger, v.type);
  EXPECT_EQ(5, tester.target->sets_called);
}

TEST(BeatTest, TestFreq)
{
  GraphTester tester(loader, Value::Type::trigger);

  tester.add("beat")
    .set("freq", 0.5)
    .connect_test("trigger", "go");

  tester.test(10);

  ASSERT_TRUE(tester.target->got("go"));
  const auto& v = tester.target->get("go");
  ASSERT_EQ(Value::Type::trigger, v.type);
  EXPECT_EQ(5, tester.target->sets_called);
}

TEST(BeatTest, TestSetInterval)
{
  GraphTester tester(loader, Value::Type::trigger);

  auto set = tester.add("set").set("value", 2);
  auto beat = tester.add("beat");

  set.connect("value", beat, "interval");
  beat.connect_test("trigger", "go");

  tester.test(10);

  ASSERT_TRUE(tester.target->got("go"));
  const auto& v = tester.target->get("go");
  ASSERT_EQ(Value::Type::trigger, v.type);
  EXPECT_EQ(5, tester.target->sets_called);
}

TEST(BeatTest, TestSetBPM)
{
  GraphTester tester(loader, Value::Type::trigger);

  auto set = tester.add("set").set("value", 30);
  auto beat = tester.add("beat");

  set.connect("value", beat, "bpm");
  beat.connect_test("trigger", "go");

  tester.test(10);

  ASSERT_TRUE(tester.target->got("go"));
  const auto& v = tester.target->get("go");
  ASSERT_EQ(Value::Type::trigger, v.type);
  EXPECT_EQ(5, tester.target->sets_called);
}

TEST(BeatTest, TestSetFreq)
{
  GraphTester tester(loader, Value::Type::trigger);

  auto set = tester.add("set").set("value", 0.5);
  auto beat = tester.add("beat");

  set.connect("value", beat, "freq");
  beat.connect_test("trigger", "go");

  tester.test(10);

  ASSERT_TRUE(tester.target->got("go"));
  const auto& v = tester.target->get("go");
  ASSERT_EQ(Value::Type::trigger, v.type);
  EXPECT_EQ(5, tester.target->sets_called);
}

TEST(BeatTest, TestIntervalWithTriggerDoesntAutoRun)
{
  GraphTester tester(loader, Value::Type::trigger);

  auto trigger = tester.add("trigger").set("wait", true);
  auto beat = tester.add("beat").set("interval", 2);

  trigger.connect("trigger", beat, "start");
  beat.connect_test("trigger", "go");

  tester.test(10);

  ASSERT_FALSE(tester.target->got("go"));
}

TEST(BeatTest, TestIntervalWithTriggeredStartRuns)
{
  GraphTester tester(loader, Value::Type::trigger);

  auto trigger = tester.add("trigger");
  auto beat = tester.add("beat").set("interval", 2);

  trigger.connect("trigger", beat, "start");
  beat.connect_test("trigger", "go");

  tester.test(10);

  ASSERT_TRUE(tester.target->got("go"));
  const auto& v = tester.target->get("go");
  ASSERT_EQ(Value::Type::trigger, v.type);
  EXPECT_EQ(5, tester.target->sets_called);
}

TEST(BeatTest, TestIntervalWithTriggeredStartAndStopRunsAndStops)
{
  GraphTester tester(loader, Value::Type::trigger);

  auto trigger = tester.add("trigger");
  auto beat1 = tester.add("beat").set("interval", 5);
  auto beat2 = tester.add("beat").set("interval", 2);

  trigger.connect("trigger", beat2, "start");
  beat1.connect("trigger", beat2, "stop");
  beat2.connect_test("trigger", "go");

  tester.test(10);

  ASSERT_TRUE(tester.target->got("go"));
  const auto& v = tester.target->get("go");
  ASSERT_EQ(Value::Type::trigger, v.type);
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
  loader.load("../trigger/vg-module-core-control-trigger.so");
  loader.load("./vg-module-core-control-beat.so");
  return RUN_ALL_TESTS();
}
