//==========================================================================
// ViGraph dataflow module: controls/velocity/test-velocity.cc
//
// Tests for <velocity> control
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module-test.h"
ModuleLoader loader;

TEST(VelocityTest, TestZeroVelocityDoesNothing)
{
  FrameGraphTester tester{loader};

  auto figure = tester.add("figure").set("points", 1);
  auto velocity = tester.add("velocity");
  auto translate = tester.add("translate");

  figure.connect("default", translate, "default");
  velocity.connect("x", translate, "x");
  velocity.connect("y", translate, "y");
  velocity.connect("z", translate, "z");

  tester.run(2);  // First tick absorbed by PV
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 2 points at 0, 0
  EXPECT_EQ(2, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(0.0, p.x);
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
  }
}

TEST(VelocityTest, TestMovement)
{
  FrameGraphTester tester{loader};

  auto figure = tester.add("figure").set("points", 1);
  auto velocity = tester.add("velocity")
    .set("x", 1)
    .set("y", 2)
    .set("z", 3)
    .set("dx", 1)
    .set("dy", 2)
    .set("dz", -0.5);
  auto translate = tester.add("translate");

  figure.connect("default", translate, "default");
  velocity.connect("x", translate, "x");
  velocity.connect("y", translate, "y");
  velocity.connect("z", translate, "z");

  tester.run(2);  // First tick absorbed by PV
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 2 points at 2,4,2.5
  EXPECT_EQ(2, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_NEAR(2.0, p.x, 1e-5);
    EXPECT_NEAR(4.0, p.y, 1e-5);
    EXPECT_NEAR(2.5, p.z, 1e-5);
  }
}

TEST(VelocityTest, TestSetVelocity)
{
  FrameGraphTester tester{loader};

  auto figure = tester.add("figure").set("points", 1);
  auto setx = tester.add("set").set("value", 1.0);
  auto sety = tester.add("set").set("value", 2.0);
  auto setz = tester.add("set").set("value", 3.0);
  auto setdx = tester.add("set").set("value", 1.0);
  auto setdy = tester.add("set").set("value", 2.0);
  auto setdz = tester.add("set").set("value", -0.5);
  auto velocity = tester.add("velocity");
  auto translate = tester.add("translate");

  figure.connect("default", translate, "default");
  setx.connect("value", velocity, "x");
  sety.connect("value", velocity, "y");
  setz.connect("value", velocity, "z");
  setdx.connect("value", velocity, "dx");
  setdy.connect("value", velocity, "dy");
  setdz.connect("value", velocity, "dz");
  velocity.connect("x", translate, "x");
  velocity.connect("y", translate, "y");
  velocity.connect("z", translate, "z");

  tester.run(2);  // First tick absorbed by PV
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 2 points at 2.0, 4.0, 2.5
  EXPECT_EQ(2, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_NEAR(2.0, p.x, 1e-5);
    EXPECT_NEAR(4.0, p.y, 1e-5);
    EXPECT_NEAR(2.5, p.z, 1e-5);
  }
}

TEST(VelocityTest, TestSetVelocityWithMax)
{
  FrameGraphTester tester{loader};

  auto figure = tester.add("figure").set("points", 1);
  auto setdx = tester.add("set").set("value", 1.0);
  auto setdy = tester.add("set").set("value", 2.0);
  auto setdz = tester.add("set").set("value", -2.0);
  auto setmax = tester.add("set").set("value", 1.5);
  auto velocity = tester.add("velocity");
  auto translate = tester.add("translate");

  figure.connect("default", translate, "default");
  setdx.connect("value", velocity, "dx");
  setdy.connect("value", velocity, "dy");
  setdz.connect("value", velocity, "dz");
  setmax.connect("value", velocity, "max");
  velocity.connect("x", translate, "x");
  velocity.connect("y", translate, "y");
  velocity.connect("z", translate, "z");

  tester.run(2);  // First tick absorbed by PV
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  // Requested magnitude is sqrt(1+4+4) = 3, max is 1.5, so
  // each axis should be halved.
  // Should be 2 points at 0.5, 1.0. -1.0
  EXPECT_EQ(2, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_NEAR(0.5, p.x, 1e-5);
    EXPECT_NEAR(1.0, p.y, 1e-5);
    EXPECT_NEAR(-1.0, p.z, 1e-5);
  }
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
  loader.load("../../sources/figure/vg-module-vector-source-figure.so");
  loader.load("../../filters/translate/vg-module-vector-filter-translate.so");
  loader.load("./vg-module-vector-control-velocity.so");
  loader.add_default_section("vector");
  return RUN_ALL_TESTS();
}
