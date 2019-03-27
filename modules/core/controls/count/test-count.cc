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
  ControlTester tester(loader);
  tester.test("<count property='foo'/>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& sp = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_EQ(1, sp.v.d);
}

TEST(CountTest, TestCountSpecified)
{
  ControlTester tester(loader);
  tester.test("<count property='foo' delta='42'/>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& sp = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_EQ(42, sp.v.d);
}

TEST(CountTest, TestCountWithWaitNotTriggeredHasNoEffect)
{
  ControlTester tester(loader);
  tester.test("<count property='foo' wait='yes'/>");
  ASSERT_FALSE(tester.target->got("foo"));
}

TEST(CountTest, TestCountWithAutoWaitNotTriggeredHasNoEffect)
{
  ControlTester tester(loader);
  tester.test("<trigger wait='yes'/>",
              "<count property='foo'/>", 2);
  ASSERT_FALSE(tester.target->got("foo"));
}

TEST(CountTest, TestCountWithWaitTriggeredHasEffect)
{
  ControlTester tester(loader);
  tester.test("<trigger/>",
              "<count property='foo' wait='yes'/>");
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
