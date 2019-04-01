//==========================================================================
// ViGraph dataflow module: core/controls/sequence/test-sequence.cc
//
// Tests for <sequence> control
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(SequenceTest, TestSequenceAttrValues)
{
  ControlTester tester(loader, Value::Type::text);
  tester.test("<set property='index' value='2'/>",
              "<sequence values='zero one two three four' />");
  ASSERT_TRUE(tester.target->got("value"));
  const auto& sp = tester.target->get("value");
  ASSERT_EQ(Value::Type::text, sp.v.type);
  EXPECT_EQ("two", sp.v.s);
}

TEST(SequenceTest, TestSequenceElementValues)
{
  ControlTester tester(loader, Value::Type::text);
  tester.test("<set property='index' value='3'/>",
              "<sequence>"
              "  <value>zero</value>"
              "  <value>one</value>"
              "  <value>two</value>"
              "  <value>three</value>"
              "  <value>four</value>"
              "</sequence>");
  ASSERT_TRUE(tester.target->got("value"));
  const auto& sp = tester.target->get("value");
  ASSERT_EQ(Value::Type::text, sp.v.type);
  EXPECT_EQ("three", sp.v.s);
}

TEST(SequenceTest, TestSequenceTriggerNext)
{
  ControlTester tester(loader, Value::Type::text);
  tester.test("<trigger target='seq' property='next'/>",
              "<trigger property='next'/>",
              "<sequence id='seq' values='zero one two three four' />");
  ASSERT_TRUE(tester.target->got("value"));
  const auto& sp = tester.target->get("value");
  ASSERT_EQ(Value::Type::text, sp.v.type);
  EXPECT_EQ("two", sp.v.s);
}

TEST(SequenceTest, TestSequenceTriggerNextLoop)
{
  ControlTester tester(loader, Value::Type::text);
  tester.test({"<trigger target='seq' property='next'/>",
               "<trigger target='seq' property='next'/>",
               "<trigger target='seq' property='next'/>",
               "<trigger target='seq' property='next'/>",
               "<trigger target='seq' property='next'/>",
               "<trigger property='next'/>",
               "<sequence id='seq' values='zero one two three four' />"});
  ASSERT_TRUE(tester.target->got("value"));
  const auto& sp = tester.target->get("value");
  ASSERT_EQ(Value::Type::text, sp.v.type);
  EXPECT_EQ("one", sp.v.s);
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
  loader.load("../trigger/vg-module-core-control-trigger.so");
  loader.load("./vg-module-core-control-sequence.so");
  return RUN_ALL_TESTS();
}
