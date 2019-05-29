//==========================================================================
// ViGraph dataflow module:
//  laser/filters/add-blanking-anchors/test-add-blanking-anchors.cc
//
// Tests for <add-blanking-anchors> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../vector/vector-module-test.h"
ModuleLoader loader;

TEST(AddBlankingAnchorsTest, TestBlankingPointInsertion)
{
  FrameGraphTester tester{loader};

  auto svg = tester.add("svg")
    .set("path", "M 0 0 L 1 0 M 2 0 L 3 0")
    .set("normalise", false);
  auto aba = tester.add("add-blanking-anchors")
    .set("leading", 1)
    .set("trailing", 2);

  svg.connect("default", aba, "default");

  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  vector<Point>& opoints = frame->points;
  ASSERT_EQ(10, opoints.size());  // 1 at start, 2 added at 1, 1 at 2, 2 at end
  int i=0;
  // added leading at start
  EXPECT_EQ(Point(0,0), opoints[i]);
  EXPECT_TRUE(opoints[i++].is_blanked());
  // original
  EXPECT_EQ(Point(0,0), opoints[i]);
  EXPECT_TRUE(opoints[i++].is_blanked());
  // added trailing
  EXPECT_EQ(Point(1,0), opoints[i]);
  EXPECT_EQ(Colour::white, opoints[i++].c);
  EXPECT_EQ(Point(1,0), opoints[i]);
  EXPECT_EQ(Colour::white, opoints[i++].c);
  // original
  EXPECT_EQ(Point(1,0), opoints[i]);
  EXPECT_EQ(Colour::white, opoints[i++].c);
  // added leading
  EXPECT_EQ(Point(2,0), opoints[i]);
  EXPECT_TRUE(opoints[i++].is_blanked());
  // original
  EXPECT_EQ(Point(2,0), opoints[i]);
  EXPECT_TRUE(opoints[i++].is_blanked());
  // original
  EXPECT_EQ(Point(3,0), opoints[i]);
  EXPECT_EQ(Colour::white, opoints[i++].c);
  // added trailing at end
  EXPECT_EQ(Point(3,0), opoints[i]);
  EXPECT_EQ(Colour::white, opoints[i++].c);
  EXPECT_EQ(Point(3,0), opoints[i]);
  EXPECT_EQ(Colour::white, opoints[i++].c);

  EXPECT_EQ(10, i);
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
  loader.load("./vg-module-laser-filter-add-blanking-anchors.so");
  loader.add_default_section("vector");
  loader.add_default_section("laser");
  return RUN_ALL_TESTS();
}
