//==========================================================================
// ViGraph dataflow module: core/controls/compare/test-compare.cc
//
// Tests for <compare> control
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(CompareTest, TestDefaultCompareTriggersInRange)
{
  GraphTester tester(loader, Value::Type::trigger);

  auto set = tester.add("set").set("value", 0.2);
  auto compare = tester.add("compare");

  set.connect(compare);
  compare.connect_test("trigger", "yes");

  tester.test();

  EXPECT_TRUE(tester.target->got("yes"));
}

TEST(CompareTest, TestDefaultCompareClearsAboveRange)
{
  GraphTester tester(loader, Value::Type::trigger);

  auto set = tester.add("set").set("value", 1.5);
  auto compare = tester.add("compare");

  set.connect(compare);
  compare.connect_test("clear", "no");

  tester.test();

  EXPECT_TRUE(tester.target->got("no"));
}

TEST(CompareTest, TestDefaultCompareClearsBelowRange)
{
  GraphTester tester(loader, Value::Type::trigger);

  auto set = tester.add("set").set("value", -0.5);
  auto compare = tester.add("compare");

  set.connect(compare);
  compare.connect_test("clear", "no");

  tester.test();

  EXPECT_TRUE(tester.target->got("no"));
}

TEST(CompareTest, TestSpecifiedCompareTriggersInRange)
{
  GraphTester tester(loader, Value::Type::trigger);

  auto set = tester.add("set").set("value", 3.5);
  auto compare = tester.add("compare").set("min", 3).set("max", 4);

  set.connect(compare);
  compare.connect_test("trigger", "yes");

  tester.test();

  EXPECT_TRUE(tester.target->got("yes"));
}

TEST(CompareTest, TestSpecifiedCompareAbsoluteValueUpper)
{
  GraphTester tester(loader, Value::Type::trigger);

  auto set = tester.add("set").set("value", 5.5);
  auto compare = tester.add("compare").set("min", 3).set("max", 4);

  set.connect(compare);
  compare.connect_test("clear", "no");

  tester.test();

  EXPECT_TRUE(tester.target->got("no"));
}

TEST(CompareTest, TestSpecifiedCompareAbsoluteValueLower)
{
  GraphTester tester(loader, Value::Type::trigger);

  auto set = tester.add("set").set("value", 0);
  auto compare = tester.add("compare").set("min", 3).set("max", 4);

  set.connect(compare);
  compare.connect_test("clear", "no");

  tester.test();

  EXPECT_TRUE(tester.target->got("no"));
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
  loader.load("./vg-module-core-control-compare.so");
  return RUN_ALL_TESTS();
}
