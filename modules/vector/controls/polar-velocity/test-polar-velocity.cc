//==========================================================================
// ViGraph dataflow module:
//   vector/controls/polar-velocity/test-polar-velocity.cc
//
// Tests for <polar-velocity> control
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module-test.h"
ModuleLoader loader;

TEST(PolarVelocityTest, TestZeroVelocityDoesNothing)
{
  // Move at 1/s vertically up
  const string& xml = R"(
    <graph>
      <figure points='1'/>
      <polar-velocity x="1" y="2" v="0" angle="0"/>
      <translate/>
    </graph>
  )";

  FrameGenerator gen(xml, loader, 2);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 2 points at 0, 0
  EXPECT_EQ(2, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(1.0, p.x);
    EXPECT_EQ(2.0, p.y);
    EXPECT_EQ(0.0, p.z);
  }
}

TEST(PolarVelocityTest, TestMovementRight)
{
  // Move at 1/s horizontally
  const string& xml = R"(
    <graph>
      <figure points='1'/>
      <polar-velocity v="1" angle="0"/>
      <translate/>
    </graph>
  )";

  FrameGenerator gen(xml, loader, 2);
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

TEST(PolarVelocityTest, TestMovementUp)
{
  // Move at 1/s vertically up
  const string& xml = R"(
    <graph>
      <figure points='1'/>
      <polar-velocity v="1" angle="0.25"/>
      <translate/>
    </graph>
  )";

  FrameGenerator gen(xml, loader, 2);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 2 points at 0, 1
  EXPECT_EQ(2, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_NEAR(0.0, p.x, 1e-5);
    EXPECT_NEAR(1.0, p.y, 1e-5);
    EXPECT_EQ(0.0, p.z);
  }
}

TEST(PolarVelocityTest, TestMovementLeft)
{
  // Move at 2/s horizontally left
  const string& xml = R"(
    <graph>
      <figure points='1'/>
      <polar-velocity v="2" angle="0.5"/>
      <translate/>
    </graph>
  )";

  FrameGenerator gen(xml, loader, 2);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 2 points at -2,0
  EXPECT_EQ(2, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_NEAR(-2.0, p.x, 1e-5);
    EXPECT_NEAR(0.0, p.y, 1e-5);
    EXPECT_EQ(0.0, p.z);
  }
}

TEST(PolarVelocityTest, TestMovementDown)
{
  // Move at 0.5/s vertically up
  const string& xml = R"(
    <graph>
      <figure points='1'/>
      <polar-velocity v="0.5" angle="0.75"/>
      <translate/>
    </graph>
  )";

  FrameGenerator gen(xml, loader, 2);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 2 points at 0, -0.5
  EXPECT_EQ(2, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_NEAR(0.0, p.x, 1e-5);
    EXPECT_NEAR(-0.5, p.y, 1e-5);
    EXPECT_EQ(0.0, p.z);
  }
}

TEST(PolarVelocityTest, TestSetAngleAndVelocity)
{
  // Move at 0.5/s vertically up
  const string& xml = R"(
    <graph>
      <figure points='1'/>
      <set target="pv" value="1.0" property="x"/>
      <set target="pv" value="2.0" property="y"/>
      <set target="pv" value="1.0" property="v"/>
      <set target="pv" value="0.25" property="angle"/>
      <polar-velocity id="pv" v="0" angle="0"/>
      <translate/>
    </graph>
  )";

  FrameGenerator gen(xml, loader, 2);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 2 points at 1, 3
  EXPECT_EQ(2, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_NEAR(1.0, p.x, 1e-5);
    EXPECT_NEAR(3.0, p.y, 1e-5);
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
  loader.load("./vg-module-vector-control-polar-velocity.so");
  return RUN_ALL_TESTS();
}
