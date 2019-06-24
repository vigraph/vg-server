//==========================================================================
// ViGraph dataflow module:
//   core/controls/trigger-sequence/test-trigger-sequence.cc
//
// Tests for <trigger-sequence> control
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(TriggerSequenceTest, TestSetIndex)
{
  GraphTester tester(loader, Value::Type::trigger);

  auto set = tester.add("set").set("value", 2);

  JSON::Value json(JSON::Value::ARRAY);
  json.add(JSON::Value::FALSE_);
  json.add(JSON::Value::FALSE_);
  json.add(JSON::Value::TRUE_);
  json.add(JSON::Value::FALSE_);
  json.add(JSON::Value::FALSE_);
  auto seq = tester.add("trigger-sequence").set("values", json);

  set.connect(seq, "index");
  seq.connect_test("trigger");

  tester.test();

  ASSERT_TRUE(tester.target->got("trigger"));
  const auto& v = tester.target->get("trigger");
  ASSERT_EQ(Value::Type::trigger, v.type);
}

TEST(TriggerSequenceTest, TestTriggerNext)
{
  GraphTester tester(loader, Value::Type::trigger);

  auto trigger1 = tester.add("trigger");
  auto trigger2 = tester.add("trigger");

  JSON::Value json(JSON::Value::ARRAY);
  json.add(JSON::Value::FALSE_);
  json.add(JSON::Value::FALSE_);
  json.add(JSON::Value::TRUE_);
  json.add(JSON::Value::FALSE_);
  json.add(JSON::Value::FALSE_);
  auto seq = tester.add("trigger-sequence").set("values", json);

  trigger1.connect(seq, "next");
  trigger2.connect(seq, "next");
  seq.connect_test("trigger");

  tester.test();

  ASSERT_TRUE(tester.target->got("trigger"));
  const auto& v = tester.target->get("trigger");
  ASSERT_EQ(Value::Type::trigger, v.type);
}

TEST(TriggerSequenceTest, TestTriggerNextLoop)
{
  GraphTester tester(loader, Value::Type::trigger);

  vector<GraphTester::ElementProxy> triggers;
  for(auto i=0; i<6; i++)
    triggers.push_back(tester.add("trigger"));

  JSON::Value json(JSON::Value::ARRAY);
  json.add(JSON::Value::FALSE_);
  json.add(JSON::Value::TRUE_);
  json.add(JSON::Value::FALSE_);
  json.add(JSON::Value::FALSE_);
  json.add(JSON::Value::FALSE_);
  auto seq = tester.add("trigger-sequence").set("values", json);

  for(auto& trigger: triggers)
    trigger.connect(seq, "next");
  seq.connect_test("trigger");

  tester.test();

  ASSERT_TRUE(tester.target->got("trigger"));
  const auto& v = tester.target->get("trigger");
  ASSERT_EQ(Value::Type::trigger, v.type);
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
  loader.load("./vg-module-core-control-trigger-sequence.so");
  return RUN_ALL_TESTS();
}
