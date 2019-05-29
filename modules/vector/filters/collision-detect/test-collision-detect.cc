//==========================================================================
// ViGraph dataflow module: vector/filters/test-collision-detect.cc
//
// Tests for <collision-detect> filter
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module-test.h"
ModuleLoader loader;

TEST(CollisionDetectTest, TestWithNoCollisionDetectorPassesThrough)
{
  FrameGraphTester tester{loader};

  auto svg = tester.add("svg").set("path", "M 0 0 L 1 0");
  auto cd = tester.add("collision-detect");

  svg.connect("default", cd, "default");

  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);
}

TEST(CollisionDetectTest, TestWithCollisionDetectorPassesThrough)
{
  FrameGraphTester tester{loader};

  tester.add("collision-detector");
  auto svg = tester.add("svg").set("path", "M 0 0 L 1 0");
  auto cd = tester.add("collision-detect");

  svg.connect("default", cd, "default");

  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);
}

TEST(CollisionDetectTest, TestOverlappingSVGsCollide)
{
  GraphTester tester{loader, Value::Type::trigger};

  tester.add("collision-detector");
  auto svg1 = tester.add("svg").set("path", "M 0 0 L 2 2")
    .set("normalise", false);
  auto cd1 = tester.add("collision-detect");
  auto svg2 = tester.add("svg").set("path", "M 1 1 L 3 3")
    .set("normalise", false);
  auto cd2 = tester.add("collision-detect");

  svg1.connect("default", cd1, "default");
  svg2.connect("default", cd2, "default");
  cd1.connect_test("trigger", "cd1t");
  cd2.connect_test("trigger", "cd2t");

  tester.test(2);  // Trigger on second pretick

  ASSERT_TRUE(tester.target->got("cd1t"));
  ASSERT_EQ(Value::Type::trigger, tester.target->get("cd1t").type);

  ASSERT_TRUE(tester.target->got("cd2t"));
  ASSERT_EQ(Value::Type::trigger, tester.target->get("cd2t").type);
}

TEST(CollisionDetectTest, TestNonOverlappingSVGsDontCollide)
{
  GraphTester tester{loader, Value::Type::trigger};

  tester.add("collision-detector");
  auto svg1 = tester.add("svg").set("path", "M 0 0 L 2 2")
    .set("normalise", false);
  auto cd1 = tester.add("collision-detect");
  auto svg2 = tester.add("svg").set("path", "M 3 3 L 4 4")
    .set("normalise", false);
  auto cd2 = tester.add("collision-detect");

  svg1.connect("default", cd1, "default");
  svg2.connect("default", cd2, "default");
  cd1.connect_test("trigger", "cd1t");
  cd2.connect_test("trigger", "cd2t");

  tester.test(2);  // Trigger on second pretick

  ASSERT_FALSE(tester.target->got("cd1t"));
  ASSERT_FALSE(tester.target->got("cd2t"));
}

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  loader.load("../../../core/controls/set/vg-module-core-control-set.so");
  loader.load("../../sources/svg/vg-module-vector-source-svg.so");
  loader.load("../../services/collision-detector/vg-module-vector-service-collision-detector.so");
  loader.load("./vg-module-vector-filter-collision-detect.so");
  loader.add_default_section("vector");
  return RUN_ALL_TESTS();
}
