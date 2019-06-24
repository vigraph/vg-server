//==========================================================================
// ViGraph dataflow module: vector/filters/test-vector-to-rgb.cc
//
// Tests for <vector-to-rgb> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module-test.h"
ModuleLoader loader;

TEST(CollisionDetectTest, TestWithBlankedGivesBlack)
{
  GraphTester tester{loader};

  auto svg = tester.add("svg").set("path", "M 0 0 L 1 0");
  auto colour = tester.add("colour");
  auto v2rgb = tester.add("vector-to-rgb");

  svg.connect("default", colour, "default");
  colour.connect("default", v2rgb, "default");
  v2rgb.connect_test("r", "r");
  v2rgb.connect_test("g", "g");
  v2rgb.connect_test("b", "b");

  tester.test();

  ASSERT_TRUE(tester.target->got("r"));
  const auto& r = tester.target->get("r");
  ASSERT_EQ(Value::Type::number, r.type);
  EXPECT_EQ(0, r.d);

  ASSERT_TRUE(tester.target->got("g"));
  const auto& g = tester.target->get("g");
  ASSERT_EQ(Value::Type::number, g.type);
  EXPECT_EQ(0, g.d);

  ASSERT_TRUE(tester.target->got("b"));
  const auto& b = tester.target->get("b");
  ASSERT_EQ(Value::Type::number, b.type);
  EXPECT_EQ(0, b.d);
}

TEST(CollisionDetectTest, TestWithColourExtractsColour)
{
  GraphTester tester{loader};

  auto svg = tester.add("svg").set("path", "M 0 0 L 1 0");
  auto colour = tester.add("colour")
    .set("r", 0.1)
    .set("g", 0.2)
    .set("b", 0.3);
  auto v2rgb = tester.add("vector-to-rgb");

  svg.connect("default", colour, "default");
  colour.connect("default", v2rgb, "default");
  v2rgb.connect_test("r", "r");
  v2rgb.connect_test("g", "g");
  v2rgb.connect_test("b", "b");

  tester.test();

  ASSERT_TRUE(tester.target->got("r"));
  const auto& r = tester.target->get("r");
  ASSERT_EQ(Value::Type::number, r.type);
  EXPECT_DOUBLE_EQ(0.1, r.d);

  ASSERT_TRUE(tester.target->got("g"));
  const auto& g = tester.target->get("g");
  ASSERT_EQ(Value::Type::number, g.type);
  EXPECT_DOUBLE_EQ(0.2, g.d);

  ASSERT_TRUE(tester.target->got("b"));
  const auto& b = tester.target->get("b");
  ASSERT_EQ(Value::Type::number, b.type);
  EXPECT_DOUBLE_EQ(0.3, b.d);
}

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  loader.load("../../sources/svg/vg-module-vector-source-svg.so");
  loader.load("../colour/vg-module-vector-filter-colour.so");
  loader.load("./vg-module-vector-filter-vector-to-rgb.so");
  loader.add_default_section("vector");
  return RUN_ALL_TESTS();
}
