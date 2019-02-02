//==========================================================================
// ViGraph dataflow module: vector/filters/clip/test-clip.cc
//
// Tests for <clip> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module-test.h"
ModuleLoader loader;

TEST(ClipTest, TestDefaultIsNoChange)
{
  const string& xml = R"(
    <graph>
      <svg path="M 0 0 L 0.5 0" normalise="false"/>
      <clip/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(2, frame->points.size());
  EXPECT_DOUBLE_EQ(0, frame->points[0].x);
  EXPECT_TRUE(frame->points[0].is_blanked());
  EXPECT_DOUBLE_EQ(0.5, frame->points[1].x);
  EXPECT_TRUE(frame->points[1].is_lit());
}

TEST(ClipTest, TestSimpleClipOutsideWithAlphaJustFades)
{
  const string& xml = R"(
    <graph>
      <svg path="M 0 0 L 2 0" normalise="false"/>
      <clip alpha="0.5"/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(2, frame->points.size());
  EXPECT_DOUBLE_EQ(0, frame->points[0].x);
  EXPECT_TRUE(frame->points[0].is_blanked());
  EXPECT_DOUBLE_EQ(2.0, frame->points[1].x);
  EXPECT_FALSE(frame->points[1].is_blanked());
  EXPECT_DOUBLE_EQ(0.5, frame->points[1].c.get_intensity());
}

TEST(ClipTest, TestSimpleClipOutsideMovesToLastAndBlanks)
{
  const string& xml = R"(
    <graph>
      <svg path="M 0 0 L 2 0" normalise="false"/>
      <clip/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(2, frame->points.size());
  EXPECT_DOUBLE_EQ(0, frame->points[0].x);
  EXPECT_TRUE(frame->points[0].is_blanked());
  EXPECT_DOUBLE_EQ(0, frame->points[1].x);
  EXPECT_TRUE(frame->points[1].is_blanked());
}

TEST(ClipTest, TestClipInsideMovesToLastAndBlanks)
{
  const string& xml = R"(
    <graph>
      <svg path="M 0 0 L 1 0" normalise="false"/>
      <clip exclude="yes">
       <min x="0.5"/>
       <max x="1.5"/>
      </clip>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(2, frame->points.size());
  EXPECT_DOUBLE_EQ(0, frame->points[0].x);
  EXPECT_TRUE(frame->points[0].is_blanked());
  EXPECT_DOUBLE_EQ(0, frame->points[1].x);
  EXPECT_TRUE(frame->points[1].is_blanked());
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
  loader.load("../../sources/svg/vg-module-vector-source-svg.so");
  loader.load("./vg-module-vector-filter-clip.so");
  return RUN_ALL_TESTS();
}
