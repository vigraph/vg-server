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

TEST(ReorderSegmentsTest, TestSegmentsReordered)
{
  // Flat line respaced to 0.1 distance
  // Remember that <svg> normalises to -0.5..0.5 square
  const string& xml = R"(
    <graph>
      <svg path="M0,0 L1,0 M3,3 L4,4 M1,1 L2,2"/>
      <reorder-segments/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  ASSERT_EQ(6, frame->points.size());

  // !!!
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
  return RUN_ALL_TESTS();
}
