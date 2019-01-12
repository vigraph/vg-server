//==========================================================================
// ViGraph dataflow module: controls/velocity/test-velocity.cc
//
// Tests for <velocity> control
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module-test.h"
ModuleLoader loader;

TEST(VelocityTest, TestZeroVelocityDoesNothing)
{
  const string& xml = R"(
    <graph>
      <figure points='1'/>
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

TEST(VelocityTest, TestMovement)
{
  // Move at 1/s horizontally
  const string& xml = R"(
    <graph>
      <figure points='1'/>
      <velocity x="1" y="2" z="-0.5"/>
      <translate/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 2 points at 1,2,-0.5
  EXPECT_EQ(2, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_NEAR(1.0, p.x, 1e-5);
    EXPECT_NEAR(2.0, p.y, 1e-5);
    EXPECT_NEAR(-0.5, p.z, 1e-5);
  }
}

TEST(VelocityTest, TestSetVelocity)
{
  // Move at 0.5/s vertically up
  const string& xml = R"(
    <graph>
      <figure points='1'/>
      <set target="v" value="1.0" property="x"/>
      <set target="v" value="2.0" property="y"/>
      <set target="v" value="-0.5" property="z"/>
      <velocity id="v"/>
      <translate/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 2 points at 1.0, 2.0, -0.5
  EXPECT_EQ(2, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_NEAR(1.0, p.x, 1e-5);
    EXPECT_NEAR(2.0, p.y, 1e-5);
    EXPECT_NEAR(-0.5, p.z, 1e-5);
  }
}

TEST(VelocityTest, TestSetVelocityWithMax)
{
  // Move at 0.5/s vertically up
  const string& xml = R"(
    <graph>
      <figure points='1'/>
      <set target="v" value="1.0" property="x"/>
      <set target="v" value="2.0" property="y"/>
      <set target="v" value="-2.0" property="z"/>
      <set target="v" value="1.5" property="max"/>
      <velocity id="v"/>
      <translate/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  // Requested magnitude is sqrt(1+4+4) = 3, max is 1.5, so
  // each axis should be halved.
  // Should be 2 points at 0.5, 1.0. -1.0
  EXPECT_EQ(2, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_NEAR(0.5, p.x, 1e-5);
    EXPECT_NEAR(1.0, p.y, 1e-5);
    EXPECT_NEAR(-1.0, p.z, 1e-5);
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
  loader.load("./vg-module-vector-control-velocity.so");
  return RUN_ALL_TESTS();
}
