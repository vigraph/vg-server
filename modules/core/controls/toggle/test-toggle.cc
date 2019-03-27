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
  ControlTester tester(loader, Value::Type::any);
  tester.test("<trigger/>",
              "<toggle />");
  ASSERT_TRUE(tester.target->got("trigger"));
}

TEST(ToggleTest, TestTwoTriggers)
{
  ControlTester tester(loader, Value::Type::any);
  tester.test("<trigger target='tog'/>",
              "<trigger/>",
              "<toggle id='tog' />");
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
