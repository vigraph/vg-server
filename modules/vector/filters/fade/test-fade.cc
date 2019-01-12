//==========================================================================
// ViGraph dataflow module: vector/filters/fade/test-fade.cc
//
// Tests for <fade> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module-test.h"
ModuleLoader loader; 

TEST(FadeTest, TestDefaultIsNoChange)
{
  const string& xml = R"(
    <graph>
      <figure points='10'/>
      <colour r="1.0" g="0.5" b="0.3"/>
      <fade/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(11, frame->points.size());
  bool first=true;
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(first?0.0:1.0, p.c.r);
    EXPECT_EQ(first?0.0:0.5, p.c.g);
    EXPECT_EQ(first?0.0:0.3, p.c.b);
    first = false;
  }
}

TEST(FadeTest, TestFadeAllChannels)
{
  const string& xml = R"(
    <graph>
      <figure points='10'/>
      <colour r="1.0" g="0.5" b="0.3"/>
      <fade all="0.5"/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(11, frame->points.size());
  bool first=true;
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(first?0.0:0.5, p.c.r);
    EXPECT_EQ(first?0.0:0.25, p.c.g);
    EXPECT_EQ(first?0.0:0.15, p.c.b);
    first = false;
  }
}

TEST(FadeTest, TestFadeIndividualChannels)
{
  const string& xml = R"(
    <graph>
      <figure points='10'/>
      <colour r="1.0" g="0.5" b="0.3"/>
      <fade r="0.7" g="0.6" b="0.5"/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(11, frame->points.size());
  bool first=true;
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(first?0.0:0.7, p.c.r);
    EXPECT_EQ(first?0.0:0.3, p.c.g);
    EXPECT_EQ(first?0.0:0.15, p.c.b);
    first = false;
  }
}

TEST(FadeTest, TestFadeAllPropertyChanged)
{
  const string& xml = R"(
    <graph>
      <figure points='10'/>
      <colour r="1.0" g="0.5" b="0.3"/>
      <set property='all' value='0.5'/>
      <fade/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(11, frame->points.size());
  bool first=true;
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(first?0.0:0.5, p.c.r);
    EXPECT_EQ(first?0.0:0.25, p.c.g);
    EXPECT_EQ(first?0.0:0.15, p.c.b);
    first = false;
  }
}

TEST(FadeTest, TestFadeIndividualPropertyChanged)
{
  const string& xml = R"(
    <graph>
      <figure points='10'/>
      <colour r="1.0" g="0.5" b="0.3"/>
      <set target='fade' property='r' value='0.7'/>
      <set target='fade' property='g' value='0.6'/>
      <set target='fade' property='b' value='0.5'/>
      <fade/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(11, frame->points.size());
  bool first=true;
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(first?0.0:0.7, p.c.r);
    EXPECT_EQ(first?0.0:0.3, p.c.g);
    EXPECT_EQ(first?0.0:0.15, p.c.b);
    first=false;
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
  loader.load("../../filters/colour/vg-module-vector-filter-colour.so");
  loader.load("./vg-module-vector-filter-fade.so");
  return RUN_ALL_TESTS();
}
