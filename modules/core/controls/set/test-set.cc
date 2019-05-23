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
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", 42);

  set.connect_test("value", "value");

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_EQ(42, v.d);
}

TEST(SetTest, TestSetWithWaitNotTriggeredHasNoEffect)
{
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", 1).set("wait", true);

  set.connect_test("value", "value");

  tester.test();

  ASSERT_FALSE(tester.target->got("value"));
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
