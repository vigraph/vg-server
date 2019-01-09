//==========================================================================
// ViGraph dataflow module: laser/filters/optimise/test-optimise.cc
//
// Tests for <optimise> filter
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../vector/vector-module-test.h"
ModuleLoader loader;

TEST(OptimiseTest, TestMaximumDistancePointInsertion)
{
  // Flat line respaced to 0.1 distance
  // Remember that <svg> normalises to -0.5..0.5 square
  const string& xml = R"(
    <graph>
      <svg path="M0,0 L1,0"/>
      <optimise max-distance="0.1"/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  ASSERT_EQ(11, frame->points.size());  // 11 fence posts, 10 rails
  double x = -0.5;
  for(const auto& p: frame->points)
  {
    EXPECT_NEAR(x, p.x, 1e-5);
    x += 0.1;
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
  }
}

TEST(OptimiseTest, TestMaximumAngleVertexPointInsertion)
{
  // Right-angle turn downwards (remembering SVG is upside down)
  const string& xml = R"(
    <graph>
      <svg path="M0,0 L1,0 1,1"/>
      <optimise max-angle="30" vertex-repeats="5"/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
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

TEST(OptimiseTest, TestMaximumAngleIgnoresSmallAngles)
{
  // Slight turn
  const string& xml = R"(
    <graph>
      <svg path="M0,0 L1,0 2,0.5"/>
      <optimise max-angle="30" vertex-repeats="5"/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  ASSERT_EQ(3, frame->points.size());  // None added
}

TEST(OptimiseTest, TestBlankingPointInsertion)
{
  const string& xml = R"(
    <graph>
      <svg path="M0,0 L1,0"/>
      <optimise blanking-repeats="3"/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  ASSERT_EQ(8, frame->points.size());  // 2 points, 3 added at each
  for(auto i=0; i<4; i++)
  {
    const auto& p = frame->points[i];
    EXPECT_EQ(-0.5, p.x);
    EXPECT_EQ(0, p.y);
    EXPECT_TRUE(p.is_blanked());
  }
  for(auto i=4; i<8; i++)
  {
    const auto& p = frame->points[i];
    EXPECT_EQ(0.5, p.x);
    EXPECT_EQ(0, p.y);
    EXPECT_TRUE(p.is_lit());
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
  loader.load("./vg-module-laser-filter-optimise.so");
  return RUN_ALL_TESTS();
}
