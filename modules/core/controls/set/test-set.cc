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
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_EQ(42, sp.v.d);
}

TEST(SetTest, TestSetWithWaitNotTriggeredHasNoEffect)
{
  ControlTester tester(loader);
  tester.test("<set property='foo' value='1' wait='yes'/>");
  ASSERT_FALSE(tester.target->got("foo"));
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
