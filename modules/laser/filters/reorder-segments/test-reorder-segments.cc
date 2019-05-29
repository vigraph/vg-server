//==========================================================================
// ViGraph dataflow module:
//  laser/filters/reorder-segments/test-reorder-segments.cc
//
// Tests for <reorder-segments> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../vector/vector-module-test.h"
ModuleLoader loader;

TEST(ReorderSegmentsTest, TestSegmentsInSensibleOrderNotReordered)
{
  FrameGraphTester tester{loader};

  // 3 segments in sensible order
  auto svg = tester.add("svg")
    .set("path", "M0,0 L1,0 M1,1 L2,2 M3,3 L4,4")
    .set("normalise", false);
  auto rs = tester.add("reorder-segments");

  svg.connect("default", rs, "default");

  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  ASSERT_EQ(6, frame->points.size());

  EXPECT_EQ(0, frame->points[0].x);
  EXPECT_EQ(1, frame->points[1].x);
  EXPECT_EQ(1, frame->points[2].x);
  EXPECT_EQ(2, frame->points[3].x);
  EXPECT_EQ(3, frame->points[4].x);
  EXPECT_EQ(4, frame->points[5].x);
}

TEST(ReorderSegmentsTest, TestSegmentsInSillyOrderReordered)
{
  FrameGraphTester tester{loader};

  // 3 segments in non-sensible order
  auto svg = tester.add("svg")
    .set("path", "M0,0 L1,0 M3,3 L4,4 M1,1 L2,2")
    .set("normalise", false);
  auto rs = tester.add("reorder-segments");

  svg.connect("default", rs, "default");

  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  ASSERT_EQ(6, frame->points.size());

  EXPECT_EQ(0, frame->points[0].x);
  EXPECT_EQ(1, frame->points[1].x);
  EXPECT_EQ(1, frame->points[2].x);
  EXPECT_EQ(2, frame->points[3].x);
  EXPECT_EQ(3, frame->points[4].x);
  EXPECT_EQ(4, frame->points[5].x);
}

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  loader.load("../../../vector/sources/svg/vg-module-vector-source-svg.so");
  loader.load("./vg-module-laser-filter-reorder-segments.so");
  loader.add_default_section("vector");
  loader.add_default_section("laser");
  return RUN_ALL_TESTS();
}
