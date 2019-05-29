//==========================================================================
// ViGraph dataflow module:
//  laser/filters/infill-lines/test-infill-lines.cc
//
// Tests for <infill-lines> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../vector/vector-module-test.h"
ModuleLoader loader;

TEST(InfillLinesTest, TestMaximumDistancePointInsertion)
{
  FrameGraphTester tester{loader};

  // Flat line respaced to 0.1 distance
  auto svg = tester.add("svg")
    .set("path", "M 0 0 L 1 0")
    .set("normalise", false);
  auto ifl = tester.add("infill-lines").set("lit", 0.1);

  svg.connect("default", ifl, "default");

  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  ASSERT_EQ(11, frame->points.size());  // 11 fence posts, 10 rails
  double x = 0;
  for(const auto& p: frame->points)
  {
    EXPECT_NEAR(x, p.x, 1e-5);
    x += 0.1;
    EXPECT_EQ(0.0, p.y);
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
  loader.load("../../../vector/sources/svg/vg-module-vector-source-svg.so");
  loader.load("./vg-module-laser-filter-infill-lines.so");
  loader.add_default_section("vector");
  loader.add_default_section("laser");
  return RUN_ALL_TESTS();
}
