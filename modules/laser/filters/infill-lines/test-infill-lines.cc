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
  // Flat line respaced to 0.1 distance
  // Remember that <svg> normalises to -0.5..0.5 square
  const string& xml = R"(
    <graph>
      <svg path="M0,0 L1,0"/>
      <laser:infill-lines lit="0.1"/>
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
  return RUN_ALL_TESTS();
}
