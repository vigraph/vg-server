//==========================================================================
// ViGraph dataflow module: vector/filters/colour/test-colour.cc
//
// Tests for <colour> filter
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module-test.h"
ModuleLoader loader;

TEST(ColourTest, TestDefaultIsBlack)
{
  const string& xml = R"(
    <graph>
      <figure points='10'/>
      <colour/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 10 points at (0,0), black
  EXPECT_EQ(10, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(0.0, p.x);
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
    EXPECT_EQ(0.0, p.c.r);
    EXPECT_EQ(0.0, p.c.g);
    EXPECT_EQ(0.0, p.c.b);
  }
}

TEST(ColourTest, TestSpecifiedRGBColour)
{
  const string& xml = R"(
    <graph>
      <figure points='10'/>
      <colour r='0.1' g='0.2' b='0.3'/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(10, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(0.0, p.x);
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
    EXPECT_EQ(0.1, p.c.r);
    EXPECT_EQ(0.2, p.c.g);
    EXPECT_EQ(0.3, p.c.b);
  }
}

TEST(ColourTest, TestSpecifiedHSLColour)
{
  const string& xml = R"(
    <graph>
      <figure points='10'/>
      <colour h='0' s='1.0' l='0.5'/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(10, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(0.0, p.x);
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
    EXPECT_EQ(1.0, p.c.r);
    EXPECT_EQ(0.0, p.c.g);
    EXPECT_EQ(0.0, p.c.b);
  }
}

TEST(ColourTest, TestSpecifiedHexColour)
{
  const string& xml = R"(
    <graph>
      <figure points='10'/>
      <colour>#00c0ff</colour>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(10, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(0.0, p.x);
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
    EXPECT_DOUBLE_EQ(0.0, p.c.r);
    EXPECT_NEAR(0.75, p.c.g, 0.01);
    EXPECT_DOUBLE_EQ(1.0, p.c.b);
  }
}

TEST(ColourTest, TestColourRGBPropertiesChanged)
{
  const string& xml = R"(
    <graph>
      <figure points='10'/>
      <set target='c' property='r' value='0.4'/>
      <set target='c' property='g' value='0.5'/>
      <set target='c' property='b' value='0.6'/>
      <colour id='c' r='0.1' g='0.2' b='0.3'/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(10, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(0.0, p.x);
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
    EXPECT_EQ(0.4, p.c.r);
    EXPECT_EQ(0.5, p.c.g);
    EXPECT_EQ(0.6, p.c.b);
  }
}

TEST(ColourTest, TestColourHSLPropertiesChanged)
{
  // Note order of sets - because colours have to be converted back
  // to RGB, if you set saturation while lightness is zero, you get back
  // grey because of conical model
  const string& xml = R"(
    <graph>
      <figure points='10'/>
      <set target='c' property='l' value='0.5'/>
      <set target='c' property='s' value='1.0'/>
      <set target='c' property='h' value='0.33333'/>
      <colour id='c' h='0' s='0' l='0'/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(10, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(0.0, p.x);
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
    EXPECT_NEAR(0.0, p.c.r, 0.01);
    EXPECT_NEAR(1.0, p.c.g, 0.01);
    EXPECT_NEAR(0.0, p.c.b, 0.01);
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
  loader.load("./vg-module-vector-filter-colour.so");
  return RUN_ALL_TESTS();
}
