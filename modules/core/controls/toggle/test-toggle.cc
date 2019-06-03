//==========================================================================
// ViGraph dataflow module: core/controls/toggle/test-toggle.cc
//
// Tests for <toggle> control
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(ToggleTest, TestOneTrigger)
{
  GraphTester tester{loader, Value::Type::trigger};

  auto trg = tester.add("trigger");
  auto tog = tester.add("toggle");

  trg.connect(tog, "toggle");
  tog.connect_test("trigger", "trigger");

  tester.test();

  ASSERT_TRUE(tester.target->got("trigger"));
}

TEST(ToggleTest, TestTwoTriggers)
{
  GraphTester tester{loader, Value::Type::trigger};

  auto trg1 = tester.add("trigger");
  auto trg2 = tester.add("trigger");
  auto tog = tester.add("toggle");

  trg1.connect(tog, "toggle");
  trg2.connect(tog, "toggle");
  tog.connect_test("clear", "clear");

  tester.test();

  ASSERT_TRUE(tester.target->got("clear"));
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
  loader.load("./vg-module-core-control-toggle.so");
  return RUN_ALL_TESTS();
}
