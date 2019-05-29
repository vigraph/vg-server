//==========================================================================
// ViGraph dataflow module: vector/filters/clip/test-clip.cc
//
// Tests for <clip> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module-test.h"
ModuleLoader loader;

TEST(ClipTest, TestDefaultIsNoChange)
{
  FrameGraphTester tester{loader};

  auto svg = tester.add("svg")
    .set("path", "M 0 0 L 0.5 0")
    .set("normalise", false);
  auto clip = tester.add("clip");

  svg.connect("default", clip, "default");

  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(2, frame->points.size());
  EXPECT_DOUBLE_EQ(0, frame->points[0].x);
  EXPECT_TRUE(frame->points[0].is_blanked());
  EXPECT_DOUBLE_EQ(0.5, frame->points[1].x);
  EXPECT_TRUE(frame->points[1].is_lit());
}

TEST(ClipTest, TestSimpleClipOutsideWithAlphaJustFades)
{
  FrameGraphTester tester{loader};

  auto svg = tester.add("svg")
    .set("path", "M 0 0 L 2 0")
    .set("normalise", false);
  auto clip = tester.add("clip")
    .set("alpha", 0.5);

  svg.connect("default", clip, "default");

  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(2, frame->points.size());
  EXPECT_DOUBLE_EQ(0, frame->points[0].x);
  EXPECT_TRUE(frame->points[0].is_blanked());
  EXPECT_DOUBLE_EQ(2.0, frame->points[1].x);
  EXPECT_FALSE(frame->points[1].is_blanked());
  EXPECT_DOUBLE_EQ(0.5, frame->points[1].c.get_intensity());
}

TEST(ClipTest, TestSimpleClipOutsideMovesToLastAndBlanks)
{
  FrameGraphTester tester{loader};

  auto svg = tester.add("svg")
    .set("path", "M 0 0 L 2 0")
    .set("normalise", false);
  auto clip = tester.add("clip");

  svg.connect("default", clip, "default");

  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(2, frame->points.size());
  EXPECT_DOUBLE_EQ(0, frame->points[0].x);
  EXPECT_TRUE(frame->points[0].is_blanked());
  EXPECT_DOUBLE_EQ(0, frame->points[1].x);
  EXPECT_TRUE(frame->points[1].is_blanked());
}

TEST(ClipTest, TestClipInsideMovesToLastAndBlanks)
{
  FrameGraphTester tester{loader};

  auto svg = tester.add("svg")
    .set("path", "M 0 0 L 1 0")
    .set("normalise", false);
  auto clip = tester.add("clip")
    .set("exclude", true)
    .set("min.x", 0.5)
    .set("max.x", 1.5);

  svg.connect("default", clip, "default");

  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(2, frame->points.size());
  EXPECT_DOUBLE_EQ(0, frame->points[0].x);
  EXPECT_TRUE(frame->points[0].is_blanked());
  EXPECT_DOUBLE_EQ(0, frame->points[1].x);
  EXPECT_TRUE(frame->points[1].is_blanked());
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
  loader.load("./vg-module-vector-filter-clip.so");
  loader.add_default_section("vector");
  return RUN_ALL_TESTS();
}
