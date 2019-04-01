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
  tester.test("<set value='0.2'/>",
              "<window/>");
  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.2, v.d, 1e-5);
}

TEST(WindowTest, TestDefaultWindowAbsoluteValueUpper)
{
  ControlTester tester(loader);
  tester.test("<set value='1.5'/>",
              "<window/>");
  EXPECT_FALSE(tester.target->got("value"));
}

TEST(WindowTest, TestDefaultWindowAbsoluteValueLower)
{
  ControlTester tester(loader);
  tester.test("<set value='-0.5'/>",
              "<window/>");
  EXPECT_FALSE(tester.target->got("value"));
}

TEST(WindowTest, TestSpecifiedWindowAbsoluteValueUpper)
{
  ControlTester tester(loader);
  tester.test("<set value='5.5'/>",
              "<window min='3' max='4'/>");
  EXPECT_FALSE(tester.target->got("value"));
}

TEST(WindowTest, TestSpecifiedWindowAbsoluteValueLower)
{
  ControlTester tester(loader);
  tester.test("<set value='-0.5'/>",
              "<window min='3' max='4'/>");
  EXPECT_FALSE(tester.target->got("value"));
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
