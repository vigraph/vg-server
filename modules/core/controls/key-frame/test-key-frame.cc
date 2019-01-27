//==========================================================================
// ViGraph dataflow module: controls/key-frame/test-key-frame.cc
//
// Tests for <key-frame> control
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(KeyFrameTest, TestKeyFrameWithNoAtsHasNoEffect)
{
  ControlTester tester(loader);
  tester.test("<key-frame property='foo'/>");
  ASSERT_FALSE(tester.target->got("foo"));
}

#if 0
TEST(KeyFrameTest, TestKeyFrameWithOneAtHasEffect)
{
  ControlTester tester(loader);
  tester.test("<key-frame property='foo'>"
                "<at foo='1'/>"
              "</key-frame>");
  ASSERT_TRUE(tester.target->got("foo"));
  const auto& sp = tester.target->get("foo");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_EQ(1, sp.v.d);
}
#endif

TEST(KeyFrameTest, TestKeyFrameWithWaitNotTriggeredHasNoEffect)
{
  ControlTester tester(loader);
  tester.test("<key-frame property='foo' wait='yes'>"
              "<at x='1'/>"
              "</key-frame>");
  ASSERT_FALSE(tester.target->got("foo"));
}

TEST(KeyFrameTest, TestKeyFrameWithAutoWaitNotTriggeredHasNoEffect)
{
  ControlTester tester(loader);
  tester.test("<set property='trigger' type='trigger' wait='yes'/>",
              "<key-frame property='foo'>"
                "<at foo='1'/>"
              "</key-frame>");
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
  loader.load("../set/vg-module-core-control-set.so");
  loader.load("./vg-module-core-control-key-frame.so");
  return RUN_ALL_TESTS();
}
