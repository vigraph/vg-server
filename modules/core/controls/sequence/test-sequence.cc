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
              "<sequence property='x' values='zero one two three four' />");
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  ASSERT_EQ(Value::Type::text, sp.v.type);
  EXPECT_EQ("two", sp.v.s);
}

TEST(SequenceTest, TestSequenceElementValues)
{
  ControlTester tester(loader, Value::Type::text);
  tester.test("<set property='index' value='3'/>",
              "<sequence property='x'>"
              "  <value>zero</value>"
              "  <value>one</value>"
              "  <value>two</value>"
              "  <value>three</value>"
              "  <value>four</value>"
              "</sequence>");
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  ASSERT_EQ(Value::Type::text, sp.v.type);
  EXPECT_EQ("three", sp.v.s);
}

TEST(SequenceTest, TestSequenceTriggerNext)
{
  ControlTester tester(loader, Value::Type::text);
  tester.test("<set target='seq' property='trigger'/>",
              "<set property='trigger'/>",
              "<sequence id='seq' property='x' "
                        "values='zero one two three four' />");
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  ASSERT_EQ(Value::Type::text, sp.v.type);
  EXPECT_EQ("two", sp.v.s);
}

TEST(SequenceTest, TestSequenceTriggerNextLoop)
{
  ControlTester tester(loader, Value::Type::text);
  tester.test({"<set target='seq' property='trigger'/>",
               "<set target='seq' property='trigger'/>",
               "<set target='seq' property='trigger'/>",
               "<set target='seq' property='trigger'/>",
               "<set target='seq' property='trigger'/>",
               "<set property='trigger'/>",
               "<sequence id='seq' property='x' "
                         "values='zero one two three four' />"});
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
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
  loader.load("./vg-module-core-control-sequence.so");
  return RUN_ALL_TESTS();
}
