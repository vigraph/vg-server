//==========================================================================
// ViGraph dataflow module:
//   vector/controls/polar-velocity/test-polar-velocity.cc
//
// Tests for <polar-velocity> control
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module-test.h"
ModuleLoader loader;

TEST(PolarVelocityTest, TestZeroVelocityDoesNothing)
{
  FrameGraphTester tester{loader};

  auto figure = tester.add("figure")
    .set("points", 1);
  auto pv = tester.add("polar-velocity")
    .set("x", 1)
    .set("y", 2)
    .set("v", 0)
    .set("angle", 0);
  auto translate = tester.add("translate");

  figure.connect("default", translate, "default");
  pv.connect("x", translate, "x");
  pv.connect("y", translate, "y");

  tester.run(2);  // First tick absorbed by PV
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 2 points at 0, 0
  EXPECT_EQ(2, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(1.0, p.x);
    EXPECT_EQ(2.0, p.y);
    EXPECT_EQ(0.0, p.z);
  }
}

TEST(PolarVelocityTest, TestMovementRight)
{
  FrameGraphTester tester{loader};

  auto figure = tester.add("figure")
    .set("points", 1);
  // Move at 1/s horizontally
  auto pv = tester.add("polar-velocity")
    .set("v", 1)
    .set("angle", 0);
  auto translate = tester.add("translate");

  figure.connect("default", translate, "default");
  pv.connect("x", translate, "x");
  pv.connect("y", translate, "y");

  tester.run(2);  // First tick absorbed by PV
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 2 points at 1,0
  EXPECT_EQ(2, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_NEAR(1.0, p.x, 1e-5);
    EXPECT_NEAR(0.0, p.y, 1e-5);
    EXPECT_EQ(0.0, p.z);
  }
}

TEST(PolarVelocityTest, TestMovementUp)
{
  FrameGraphTester tester{loader};

  auto figure = tester.add("figure")
    .set("points", 1);
  // Move at 1/s vertically up
  auto pv = tester.add("polar-velocity")
    .set("v", 1)
    .set("angle", 0.25);
  auto translate = tester.add("translate");

  figure.connect("default", translate, "default");
  pv.connect("x", translate, "x");
  pv.connect("y", translate, "y");

  tester.run(2);  // First tick absorbed by PV
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 2 points at 0, 1
  EXPECT_EQ(2, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_NEAR(0.0, p.x, 1e-5);
    EXPECT_NEAR(1.0, p.y, 1e-5);
    EXPECT_EQ(0.0, p.z);
  }
}

TEST(PolarVelocityTest, TestMovementLeft)
{
  FrameGraphTester tester{loader};

  auto figure = tester.add("figure")
    .set("points", 1);
  // Move at 2/s horizontally left
  auto pv = tester.add("polar-velocity")
    .set("v", 2)
    .set("angle", 0.5);
  auto translate = tester.add("translate");

  figure.connect("default", translate, "default");
  pv.connect("x", translate, "x");
  pv.connect("y", translate, "y");

  tester.run(2);  // First tick absorbed by PV
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 2 points at -2,0
  EXPECT_EQ(2, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_NEAR(-2.0, p.x, 1e-5);
    EXPECT_NEAR(0.0, p.y, 1e-5);
    EXPECT_EQ(0.0, p.z);
  }
}

TEST(PolarVelocityTest, TestMovementDown)
{
  FrameGraphTester tester{loader};

  auto figure = tester.add("figure")
    .set("points", 1);
  // Move at 0.5/s vertically down
  auto pv = tester.add("polar-velocity")
    .set("v", 0.5)
    .set("angle", 0.75);
  auto translate = tester.add("translate");

  figure.connect("default", translate, "default");
  pv.connect("x", translate, "x");
  pv.connect("y", translate, "y");

  tester.run(2);  // First tick absorbed by PV
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 2 points at 0, -0.5
  EXPECT_EQ(2, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_NEAR(0.0, p.x, 1e-5);
    EXPECT_NEAR(-0.5, p.y, 1e-5);
    EXPECT_EQ(0.0, p.z);
  }
}

TEST(PolarVelocityTest, TestSetAngleAndVelocity)
{
  FrameGraphTester tester{loader};

  auto figure = tester.add("figure")
    .set("points", 1);

  // Move at 0.5/s vertically up
  auto setx = tester.add("set").set("value", 1.0);
  auto sety = tester.add("set").set("value", 2.0);
  auto setv = tester.add("set").set("value", 1.0);
  auto seta = tester.add("set").set("value", 0.25);
  auto pv = tester.add("polar-velocity");
  auto translate = tester.add("translate");

  figure.connect("default", translate, "default");
  setx.connect(pv, "x");
  sety.connect(pv, "y");
  setv.connect(pv, "v");
  seta.connect(pv, "angle");
  pv.connect("x", translate, "x");
  pv.connect("y", translate, "y");

  tester.run(2);  // First tick absorbed by PV
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 2 points at 1, 3
  EXPECT_EQ(2, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_NEAR(1.0, p.x, 1e-5);
    EXPECT_NEAR(3.0, p.y, 1e-5);
    EXPECT_EQ(0.0, p.z);
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
  loader.load("./vg-module-vector-control-polar-velocity.so");
  loader.add_default_section("vector");
  return RUN_ALL_TESTS();
}
