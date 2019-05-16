//==========================================================================
// ViGraph dataflow module: core/controls/count/test-count.cc
//
// Tests for <count> control
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(CountTest, TestCountDefault)
{
  GraphTester tester(loader);

  tester.add("count").connect_test("value", "foo");

  tester.test();

  ASSERT_TRUE(tester.target->got("foo"));
  const auto& v = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(1, v.d);
}

TEST(CountTest, TestCountSpecified)
{
  GraphTester tester(loader);

  tester.add("count")
    .set("delta", 42)
    .connect_test("value", "foo");

  tester.test();

  ASSERT_TRUE(tester.target->got("foo"));
  const auto& v = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(42, v.d);
}

TEST(CountTest, TestCountWithWaitNotTriggeredHasNoEffect)
{
  GraphTester tester(loader);

  tester.add("count")
    .set("wait", true)
    .connect_test("value", "foo");

  tester.test();

  ASSERT_FALSE(tester.target->got("foo"));
}

TEST(CountTest, TestCountWithAutoWaitNotTriggeredHasNoEffect)
{
  GraphTester tester(loader);

  auto trigger = tester.add("trigger").set("wait", true);
  auto count = tester.add("count");

  trigger.connect("trigger", count, "trigger");
  count.connect_test("value", "foo");

  tester.test();

  ASSERT_FALSE(tester.target->got("foo"));
}

TEST(CountTest, TestCountWithWaitTriggeredHasEffect)
{
  GraphTester tester(loader);

  auto trigger = tester.add("trigger");
  auto count = tester.add("count");

  trigger.connect("trigger", count, "trigger");
  count.connect_test("value", "foo");

  tester.test();

  ASSERT_TRUE(tester.target->got("foo"));
}

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  loader.load("./vg-module-core-control-count.so");
  loader.load("../trigger/vg-module-core-control-trigger.so");
  return RUN_ALL_TESTS();
}
