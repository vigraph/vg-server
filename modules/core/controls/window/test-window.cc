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
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", 0.2);
  auto wnd = tester.add("window");

  set.connect(wnd);
  wnd.connect_test();

  tester.test();

  ASSERT_TRUE(tester.target->got("value"));
  const auto& v = tester.target->get("value");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.2, v.d, 1e-5);
}

TEST(WindowTest, TestDefaultWindowAbsoluteValueUpper)
{
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", 1.5);
  auto wnd = tester.add("window");

  set.connect(wnd);
  wnd.connect_test();

  tester.test();

  EXPECT_FALSE(tester.target->got("value"));
}

TEST(WindowTest, TestDefaultWindowAbsoluteValueLower)
{
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", -0.5);
  auto wnd = tester.add("window");

  set.connect(wnd);
  wnd.connect_test();

  tester.test();

  EXPECT_FALSE(tester.target->got("value"));
}

TEST(WindowTest, TestSpecifiedWindowAbsoluteValueUpper)
{
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", 5.5);
  auto wnd = tester.add("window").set("min", 3).set("max", 4);

  set.connect(wnd);
  wnd.connect_test();

  tester.test();

  EXPECT_FALSE(tester.target->got("value"));
}

TEST(WindowTest, TestSpecifiedWindowAbsoluteValueLower)
{
  GraphTester tester{loader};

  auto set = tester.add("set").set("value", -0.5);
  auto wnd = tester.add("window").set("min", 3).set("max", 4);

  set.connect(wnd);
  wnd.connect_test();

  tester.test();

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
