//==========================================================================
// ViGraph dataflow module: core/controls/modify/test-modify.cc
//
// Tests for <modify> control
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(ModifyTest, TestModifyDefault)
{
  ControlTester tester(loader);
  tester.test("<modify property='foo'/>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& sp = tester.target->get("foo");
  EXPECT_TRUE(sp.increment);
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_EQ(1, sp.v.d);
}

TEST(ModifyTest, TestModifySpecified)
{
  ControlTester tester(loader);
  tester.test("<modify property='foo' delta='42'/>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& sp = tester.target->get("foo");
  EXPECT_TRUE(sp.increment);
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_EQ(42, sp.v.d);
}

TEST(ModifyTest, TestModifyWithWaitNotTriggeredHasNoEffect)
{
  ControlTester tester(loader);
  tester.test("<modify property='foo' wait='yes'/>");
  ASSERT_FALSE(tester.target->got("foo"));
}

TEST(ModifyTest, TestModifyWithAutoWaitNotTriggeredHasNoEffect)
{
  ControlTester tester(loader);
  tester.test("<set property='trigger' type='trigger' wait='yes'/>",
              "<modify property='foo'/>", 2);
  ASSERT_FALSE(tester.target->got("foo"));
}

TEST(ModifyTest, TestModifyWithWaitTriggeredHasEffect)
{
  ControlTester tester(loader);
  tester.test("<set property='trigger' type='trigger'/>",
              "<modify property='foo' wait='yes'/>", 2);
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
  loader.load("./vg-module-core-control-modify.so");
  loader.load("../set/vg-module-core-control-set.so");
  return RUN_ALL_TESTS();
}
