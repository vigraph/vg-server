//==========================================================================
// ViGraph dataflow module: controls/polar-acceleration/test-polar-acceleration.cc
//
// Tests for <polar-acceleration> control
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module-test.h"
ModuleLoader loader;

TEST(PolarAccelerationTest, TestZeroAccelerationDoesNothing)
{
  const string& xml = R"(
    <graph>
      <figure points='1'/>
      <polar-acceleration a="0" angle="0"/>
      <velocity/>
      <translate/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 2 points at 0, 0
  EXPECT_EQ(2, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(0.0, p.x);
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
  }
}

TEST(PolarAccelerationTest, TestAccelerationRight)
{
  // Move at 1/s horizontally
  const string& xml = R"(
    <graph>
      <figure points='1'/>
      <polar-acceleration a="1" angle="0"/>
      <velocity/>
      <translate/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 2 points at 1,0
  EXPECT_EQ(2, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_NEAR(1.0, p.x, 1e-5);
    EXPECT_NEAR(0.0, p.y, 1e-5);
    EXPECT_EQ(0.0, p.z);
  }
}

TEST(PolarAccelerationTest, TestAccelerationUp)
{
  // Move at 1/s horizontally
  const string& xml = R"(
    <graph>
      <figure points='1'/>
      <polar-acceleration a="1" angle="0.25"/>
      <velocity/>
      <translate/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 2 points at 0,1
  EXPECT_EQ(2, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_NEAR(0.0, p.x, 1e-5);
    EXPECT_NEAR(1.0, p.y, 1e-5);
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
  loader.load("../../../core/controls/set/vg-module-core-control-set.so");
  loader.load("../../sources/figure/vg-module-vector-source-figure.so");
  loader.load("../../filters/translate/vg-module-vector-filter-translate.so");
  loader.load("../../controls/velocity/vg-module-vector-control-velocity.so");
  loader.load("./vg-module-vector-control-polar-acceleration.so");
  return RUN_ALL_TESTS();
}
