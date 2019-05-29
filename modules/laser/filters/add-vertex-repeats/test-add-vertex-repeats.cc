//==========================================================================
// ViGraph dataflow module:
//  laser/filters/add-vertex-repeats/test-add-vertex-repeats.cc
//
// Tests for <add-vertex-repeats> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../vector/vector-module-test.h"
ModuleLoader loader;

TEST(AddVertexRepeatsTest, TestMaximumAngleVertexPointInsertion)
{
  FrameGraphTester tester{loader};

  // Right-angle turn downwards (remembering SVG is upside down)
  auto svg = tester.add("svg").set("path", "M 0,0 L 1,0 1,1");
  auto avr = tester.add("add-vertex-repeats")
    .set("max-angle", "30")
    .set("repeats", 5);

  svg.connect("default", avr, "default");

  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  ASSERT_EQ(8, frame->points.size());  // 3 points, 5 added at vertex
  for(auto i=1; i<7; i++)
  {
    const auto& p = frame->points[i];
    // Should all be at the top right corner
    EXPECT_NEAR(0.5, p.x, 1e-5);
    EXPECT_NEAR(0.5, p.y, 1e-5);
  }
}

TEST(AddVertexRepeatsTest, TestMaximumAngleIgnoresSmallAngles)
{
  FrameGraphTester tester{loader};

  // Slight turn
  auto svg = tester.add("svg").set("path", "M 0,0 L 1,0 2,0.5");
  auto avr = tester.add("add-vertex-repeats")
    .set("max-angle", "30")
    .set("repeats", 5);

  svg.connect("default", avr, "default");

  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  ASSERT_EQ(3, frame->points.size());  // None added
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
  loader.load("./vg-module-laser-filter-add-vertex-repeats.so");
  loader.add_default_section("vector");
  loader.add_default_section("laser");
  return RUN_ALL_TESTS();
}
