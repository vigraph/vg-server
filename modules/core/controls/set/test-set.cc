//==========================================================================
// ViGraph dataflow module: controls/set/test-set.cc
//
// Tests for <set> control
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(SetTest, TestAbsoluteValueSet)
{
  ControlTester tester(loader);
  tester.test("<set property='foo' value='42'/>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& sp = tester.target->get("foo");
  EXPECT_FALSE(sp.increment);
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_EQ(42, sp.v.d);
}

TEST(SetTest, TestAbsoluteValueSetTrigger)
{
  ControlTester tester(loader, Value::Type::trigger);
  tester.test("<set property='foo' type='trigger'/>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& sp = tester.target->get("foo");
  EXPECT_FALSE(sp.increment);
  EXPECT_EQ(Value::Type::trigger, sp.v.type);
}

TEST(SetTest, TestSetWithWaitNotTriggeredHasNoEffect)
{
  ControlTester tester(loader);
  tester.test("<set property='foo' value='1' wait='yes'/>");
  ASSERT_FALSE(tester.target->got("foo"));
}

TEST(SetTest, TestSetWithAutoWaitNotTriggeredHasNoEffect)
{
  ControlTester tester(loader);
  tester.test("<set property='trigger' type='trigger' wait='yes'/>",
              "<set property='foo' value='1'/>", 2);
  ASSERT_FALSE(tester.target->got("foo"));
}

TEST(SetTest, TestSetWithWaitTriggeredHasEffect)
{
  ControlTester tester(loader);
  tester.test("<set property='trigger' type='trigger'/>",
              "<set property='foo' value='1' wait='yes'/>", 2);
  ASSERT_TRUE(tester.target->got("foo"));
}

TEST(SetTest, TestTriggerNotDelayedIfDelayNotSet)
{
  ControlTester tester(loader, Value::Type::trigger);
  tester.test("<set type='trigger' property='trigger'/>",
              "<set type='trigger' property='trigger'/>", 1);
  EXPECT_TRUE(tester.target->got("trigger"));
}

TEST(SetTest, TestZeroDelayTriggersNextTick)
{
  ControlTester tester(loader, Value::Type::trigger);
  tester.test("<set type='trigger' property='trigger'/>",
              "<set delay='0' type='trigger' property='trigger'/>", 2);
  EXPECT_TRUE(tester.target->got("trigger"));
}

TEST(SetTest, TestSetDelayDoesntTriggerEarly)
{
  ControlTester tester(loader, Value::Type::trigger);
  tester.test("<set type='trigger' property='trigger'/>",
              "<set delay='2' type='trigger' property='trigger'/>", 2);
  EXPECT_FALSE(tester.target->got("trigger"));
}

TEST(SetTest, TestSetDelayTriggers)
{
  ControlTester tester(loader, Value::Type::trigger);
  tester.test("<set type='trigger' property='trigger'/>",
              "<set delay='2' type='trigger' property='trigger'/>", 3);
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
  loader.load("./vg-module-core-control-set.so");
  return RUN_ALL_TESTS();
}
