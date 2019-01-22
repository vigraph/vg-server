//==========================================================================
// ViGraph dataflow module: vector/filters/pattern/test-pattern.cc
//
// Tests for <pattern> filter
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module-test.h"
ModuleLoader loader;

TEST(PatternTest, TestDefaultHasNoEffect)
{
  const string& xml = R"(
    <graph>
      <figure points='10'/>
      <pattern/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 11 points at (0,0), first blanked, rest white
  EXPECT_EQ(11, frame->points.size());
  EXPECT_TRUE(frame->points[0].is_blanked());
  for(auto i=1u; i<frame->points.size(); i++)
  {
    const auto&p = frame->points[i];
    EXPECT_EQ(0.0, p.x);
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
    EXPECT_EQ(1.0, p.c.r);
    EXPECT_EQ(1.0, p.c.g);
    EXPECT_EQ(1.0, p.c.b);
  }
}

TEST(PatternTest, TestSimpleAlternatingOneRepeat)
{
  const string& xml = R"(
    <graph>
      <figure points='10'/>
      <pattern>
        <colour>red</colour>
        <colour g="1.0"/>
      </pattern>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 11 points at (0,0), first blanked, 5 red, 5 green
  EXPECT_EQ(11, frame->points.size());
  EXPECT_TRUE(frame->points[0].is_blanked());
  for(auto i=1; i<6; i++)
  {
    const auto&p = frame->points[i];
    EXPECT_EQ(1.0, p.c.r) << i;
    EXPECT_EQ(0.0, p.c.g) << i;
    EXPECT_EQ(0.0, p.c.b) << i;
  }
  for(auto i=6; i<11; i++)
  {
    const auto&p = frame->points[i];
    EXPECT_EQ(0.0, p.c.r) << i;
    EXPECT_EQ(1.0, p.c.g) << i;
    EXPECT_EQ(0.0, p.c.b) << i;
  }
}

TEST(PatternTest, TestSimpleAlternating5Repeats)
{
  const string& xml = R"(
    <graph>
      <figure points='10'/>
      <pattern repeats="5">
        <colour>#ffff00</colour>
        <colour b="1.0"/>
      </pattern>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 11 points at (0,0), first blanked, rest alternating yellow, blue
  EXPECT_EQ(11, frame->points.size());
  EXPECT_TRUE(frame->points[0].is_blanked());
  for(auto i=1; i<11; i++)
  {
    const auto&p = frame->points[i];
    if (i&1) // even because starting at 1
    {
      EXPECT_EQ(1.0, p.c.r) << i;
      EXPECT_EQ(1.0, p.c.g) << i;
      EXPECT_EQ(0.0, p.c.b) << i;
    }
    else
    {
      EXPECT_EQ(0.0, p.c.r) << i;
      EXPECT_EQ(0.0, p.c.g) << i;
      EXPECT_EQ(1.0, p.c.b) << i;
    }
  }
}

TEST(PatternTest, TestSimpleAlternating5RepeatsAntiPhase)
{
  const string& xml = R"(
    <graph>
      <figure points='10'/>
      <pattern repeats="5" phase="0.5">
        <colour>#ffff00</colour>
        <colour b="1.0"/>
      </pattern>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 11 points at (0,0), first blanked, rest alternating yellow, blue
  EXPECT_EQ(11, frame->points.size());
  EXPECT_TRUE(frame->points[0].is_blanked());
  for(auto i=1; i<11; i++)
  {
    const auto&p = frame->points[i];
    if (i&1) // even because starting at 1
    {
      EXPECT_EQ(0.0, p.c.r) << i;
      EXPECT_EQ(0.0, p.c.g) << i;
      EXPECT_EQ(1.0, p.c.b) << i;
    }
    else
    {
      EXPECT_EQ(1.0, p.c.r) << i;
      EXPECT_EQ(1.0, p.c.g) << i;
      EXPECT_EQ(0.0, p.c.b) << i;
    }
  }
}

TEST(PatternTest, TestBlendOneRepeat)
{
  const string& xml = R"(
    <graph>
      <figure points='10'/>
      <pattern blend="linear">
        <colour>red</colour>
        <colour g="1.0"/>
      </pattern>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 11 points at (0,0), first blanked,
  // rest blending between pure red and pure green and back again
  EXPECT_EQ(11, frame->points.size());
  EXPECT_TRUE(frame->points[0].is_blanked());
  for(auto i=1; i<6; i++)
  {
    const auto&p = frame->points[i];
    EXPECT_DOUBLE_EQ(1-(i-1)/5.0, p.c.r) << i;
    EXPECT_DOUBLE_EQ((i-1)/5.0, p.c.g) << i;
    EXPECT_DOUBLE_EQ(0.0, p.c.b) << i;
  }
  for(auto i=6; i<11; i++)
  {
    const auto&p = frame->points[i];
    EXPECT_DOUBLE_EQ((i-6)/5.0, p.c.r) << i;
    EXPECT_DOUBLE_EQ(1-(i-6)/5.0, p.c.g) << i;
    EXPECT_DOUBLE_EQ(0.0, p.c.b) << i;
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
  loader.load("./vg-module-vector-filter-pattern.so");
  return RUN_ALL_TESTS();
}
