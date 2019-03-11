//==========================================================================
// ViGraph dataflow module: core/controls/window/test-window.cc
//
// Tests for <window> control
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(WindowTest, TestWindowDoesNothingInRange)
{
  ControlTester tester(loader);
  tester.test("<set property='x' value='0.2'/>",
              "<window property='x'/>");
  ASSERT_TRUE(tester.target->got("x"));
  const auto& sp = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, sp.v.type);
  EXPECT_NEAR(0.2, sp.v.d, 1e-5);
}

TEST(WindowTest, TestDefaultWindowAbsoluteValueUpper)
{
  ControlTester tester(loader);
  tester.test("<set property='x' value='1.5'/>",
              "<window property='x'/>");
  EXPECT_FALSE(tester.target->got("x"));
}

TEST(WindowTest, TestDefaultWindowAbsoluteValueLower)
{
  ControlTester tester(loader);
  tester.test("<set property='x' value='-0.5'/>",
              "<window property='x'/>");
  EXPECT_FALSE(tester.target->got("x"));
}

TEST(WindowTest, TestSpecifiedWindowAbsoluteValueUpper)
{
  ControlTester tester(loader);
  tester.test("<set property='x' value='5.5'/>",
              "<window property='x' min='3' max='4'/>");
  EXPECT_FALSE(tester.target->got("x"));
}

TEST(WindowTest, TestSpecifiedWindowAbsoluteValueLower)
{
  ControlTester tester(loader);
  tester.test("<set property='x' value='-0.5'/>",
              "<window property='x' min='3' max='4'/>");
  EXPECT_FALSE(tester.target->got("x"));
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
  loader.load("./vg-module-core-control-window.so");
  return RUN_ALL_TESTS();
}