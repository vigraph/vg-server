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

  auto add = tester.add("add")
    .connect_test("value", "x");

  tester.add("set")
    .set("value", 0.2)
    .connect("value", add, "value");

  tester.test();

  ASSERT_TRUE(tester.target->got("x"));
  const auto& v = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.2, v.d, 1e-5);
}

TEST(AddTest, TestAddValue)
{
  GraphTester tester(loader);

  auto add = tester.add("add")
    .set("offset", 0.13)
    .connect_test("value", "x");

  tester.add("set")
    .set("value", 0.2)
    .connect("value", add, "value");

  tester.test();

  const auto& v = tester.target->get("x");
  ASSERT_EQ(Value::Type::number, v.type);
  EXPECT_NEAR(0.33, v.d, 1e-5);
}

TEST(AddTest, TestModifyAddOffset)
{
  GraphTester tester(loader);

  auto add = tester.add("add")
    .connect_test("value", "x");

  tester.add("set")
    .set("value", 0.2)
    .connect("value", add, "value");

  tester.add("set")
    .set("value", 0.3)
    .connect("value", add, "offset");

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
