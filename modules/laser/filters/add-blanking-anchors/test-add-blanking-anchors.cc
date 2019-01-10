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
  const string& xml = R"(
    <graph>
      <svg path="M0,0 L1,0"/>
      <add-blanking-anchors repeats="3"/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  /* !!! FIXME
  ASSERT_EQ(5, frame->points.size());  // 2 points, 3 added at blank
  for(auto i=0; i<4; i++)
  {
    const auto& p = frame->points[i];
    EXPECT_EQ(-0.5, p.x);
    EXPECT_EQ(0, p.y);
    EXPECT_TRUE(p.is_blanked());
  }

  const auto& p = frame->points[4];
  EXPECT_EQ(0.5, p.x);
  EXPECT_EQ(0, p.y);
  EXPECT_TRUE(p.is_lit());
  */
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
  return RUN_ALL_TESTS();
}
