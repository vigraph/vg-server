//==========================================================================
// ViGraph dataflow module: controls/random/test-random.cc
//
// Tests for <random> control
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(RandomTest, TestDefaultMinMax)
{
  for(int i=0; i<100; i++)
  {
    ControlTester tester(loader);
    tester.test("<random property='foo'/>");
    ASSERT_TRUE(tester.target->got("foo"));
    const auto& sp = tester.target->get("foo");
    ASSERT_EQ(Value::Type::number, sp.v.type);
    EXPECT_LE(0.0, sp.v.d);
    EXPECT_GE(1.0, sp.v.d);
  }
}

TEST(RandomTest, TestSpecifiedMinMax)
{
  for(int i=0; i<100; i++)
  {
    ControlTester tester(loader);
    tester.test("<random property='foo' min='0.4' max='0.6'/>");
    ASSERT_TRUE(tester.target->got("foo"));
    const auto& sp = tester.target->get("foo");
    ASSERT_EQ(Value::Type::number, sp.v.type);
    EXPECT_LE(0.4, sp.v.d);
    EXPECT_GE(0.6, sp.v.d);
  }
}

TEST(RandomTest, TestRandomWithWaitNotTriggeredHasNoEffect)
{
  ControlTester tester(loader);
  tester.test("<random property='foo' wait='yes'/>");
  ASSERT_FALSE(tester.target->got("foo"));
}

TEST(RandomTest, TestRandomWithAutoWaitNotTriggeredHasNoEffect)
{
  ControlTester tester(loader);
  tester.test("<trigger wait='yes'/>",
              "<random property='foo'/>", 2);
  ASSERT_FALSE(tester.target->got("foo"));
}

TEST(RandomTest, TestRandomWithWaitTriggeredHasEffect)
{
  ControlTester tester(loader);
  tester.test("<trigger/>",
              "<random property='foo' wait='yes'/>", 2);
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
  loader.load("../trigger/vg-module-core-control-trigger.so");
  loader.load("./vg-module-core-control-random.so");
  return RUN_ALL_TESTS();
}
