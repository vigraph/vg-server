//==========================================================================
// ViGraph dataflow module: core/controls/sequence/test-sequence.cc
//
// Tests for <sequence> control
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(SequenceTest, TestSequenceSetIndex)
{
  GraphTester tester(loader, Value::Type::text);

  auto set = tester.add("set").set("value", 2);

  JSON::Value json(JSON::Value::ARRAY);
  json.add("zero");
  json.add("one");
  json.add("two");
  json.add("three");
  json.add("four");
  auto seq = tester.add("sequence").set("values", json);

  set.connect("value", seq, "index");
  seq.connect_test("value", "value");

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::text, v.type);
  EXPECT_EQ("two", v.s);
}

TEST(SequenceTest, TestSequenceTriggerNext)
{
  GraphTester tester(loader, Value::Type::text);

  auto trigger1 = tester.add("trigger");
  auto trigger2 = tester.add("trigger");

  JSON::Value json(JSON::Value::ARRAY);
  json.add("zero");
  json.add("one");
  json.add("two");
  json.add("three");
  json.add("four");
  auto seq = tester.add("sequence").set("values", json);

  trigger1.connect("trigger", seq, "next");
  trigger2.connect("trigger", seq, "next");
  seq.connect_test("value", "value");

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::text, v.type);
  EXPECT_EQ("two", v.s);
}

TEST(SequenceTest, TestSequenceTriggerNextLoop)
{
  GraphTester tester(loader, Value::Type::text);

  vector<GraphTester::ElementProxy> triggers;
  for(auto i=0; i<6; i++)
    triggers.push_back(tester.add("trigger"));

  JSON::Value json(JSON::Value::ARRAY);
  json.add("zero");
  json.add("one");
  json.add("two");
  json.add("three");
  json.add("four");
  auto seq = tester.add("sequence").set("values", json);

  for(auto& trigger: triggers)
    trigger.connect("trigger", seq, "next");
  seq.connect_test("value", "value");

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::text, v.type);
  EXPECT_EQ("one", v.s);
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
