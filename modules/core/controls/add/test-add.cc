//==========================================================================
// ViGraph dataflow module: core/controls/add/test-add.cc
//
// Tests for <add> control
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../module-test.h"
ModuleLoader loader;

TEST(AddTest, TestAddZeroDoesNothing)
{
  GraphTester tester(loader);

  auto set = tester.add("set").set("value", 0.2);
  auto add = tester.add("add");

  set.connect("value", add, "value");
  add.connect_test("value", "x");

  tester.test();

  ASSERT_TRUE(tester.target->got("x"));
  const auto& v = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.2, v.d, 1e-5);
}

TEST(AddTest, TestAddValue)
{
  GraphTester tester(loader);

  auto set = tester.add("set").set("value", 0.2);
  auto add = tester.add("add").set("offset", 0.13);

  set.connect("value", add, "value");
  add.connect_test("value", "x");

  tester.test();

  const auto& v = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.33, v.d, 1e-5);
}

TEST(AddTest, TestModifyAddOffset)
{
  GraphTester tester(loader);

  auto set1 = tester.add("set").set("value", 0.2);
  auto set2 = tester.add("set").set("value", 0.3);
  auto add = tester.add("add");

  set1.connect("value", add, "value");
  set2.connect("value", add, "offset");
  add.connect_test("value", "x");

  tester.test();

  ASSERT_TRUE(tester.target->got("x"));
  const auto& v = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.5, v.d, 1e-5);
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
  loader.load("./vg-module-core-control-add.so");
  return RUN_ALL_TESTS();
}
