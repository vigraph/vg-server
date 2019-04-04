//==========================================================================
// ViGraph dataflow module: core/controls/compare/test-compare.cc
//
// Tests for <compare> control
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(CompareTest, TestCompareDoesNothingInRange)
{
  ControlTester tester(loader, Value::Type::trigger);
  tester.test("<set value='0.2'/>",
              "<compare/>");
  EXPECT_TRUE(tester.target->got("trigger"));
}

TEST(CompareTest, TestDefaultCompareAbsoluteValueUpper)
{
  ControlTester tester(loader, Value::Type::trigger);
  tester.test("<set value='1.5'/>",
              "<compare property-clear='clear'/>");
  EXPECT_TRUE(tester.target->got("clear"));
}

TEST(CompareTest, TestDefaultCompareAbsoluteValueLower)
{
  ControlTester tester(loader, Value::Type::trigger);
  tester.test("<set value='-0.5'/>",
              "<compare property-clear='clear'/>");
  EXPECT_TRUE(tester.target->got("clear"));
}

TEST(CompareTest, TestSpecifiedCompareAbsoluteValueUpper)
{
  ControlTester tester(loader, Value::Type::trigger);
  tester.test("<set value='5.5'/>",
              "<compare min='3' max='4' property-clear='clear'/>");
  EXPECT_TRUE(tester.target->got("clear"));
}

TEST(CompareTest, TestSpecifiedCompareAbsoluteValueLower)
{
  ControlTester tester(loader, Value::Type::trigger);
  tester.test("<set value='-0.5'/>",
              "<compare min='3' max='4' property-clear='clear'/>");
  EXPECT_TRUE(tester.target->got("clear"));
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
