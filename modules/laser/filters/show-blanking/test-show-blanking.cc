//==========================================================================
// ViGraph dataflow module: laser/filters/show-blanking/test-show-blanking.cc
//
// Tests for <show-blanking> filter
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "../../../vector/vector-module-test.h"
ModuleLoader loader;

TEST(ShowBlankingTest, TestBlankingIsRedAndLitPointsLeftAlone)
{
  const string& xml = R"(
    <graph>
      <svg path="M0,0 L1,0"/>
      <laser:show-blanking colour="red"/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  ASSERT_EQ(2, frame->points.size());
  EXPECT_EQ(Colour::red, frame->points[0].c);
  EXPECT_EQ(Colour::white, frame->points[1].c);
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
  loader.load("./vg-module-laser-filter-show-blanking.so");
  return RUN_ALL_TESTS();
}
