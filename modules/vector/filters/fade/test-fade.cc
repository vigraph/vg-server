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
  FrameGraphTester tester{loader};

  auto figure = tester.add("figure").set("points", 10);
  auto colour = tester.add("colour")
    .set("r", 1.0)
    .set("g", 0.5)
    .set("b", 0.3);
  auto fade = tester.add("fade");

  figure.connect("default", colour, "default");
  colour.connect("default", fade, "default");

  tester.run();
  Frame *frame = tester.get_frame();
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
  FrameGraphTester tester{loader};

  auto figure = tester.add("figure").set("points", 10);
  auto colour = tester.add("colour")
    .set("r", 1.0)
    .set("g", 0.5)
    .set("b", 0.3);
  auto fade = tester.add("fade")
    .set("all", 0.5);

  figure.connect("default", colour, "default");
  colour.connect("default", fade, "default");

  tester.run();
  Frame *frame = tester.get_frame();
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
  FrameGraphTester tester{loader};

  auto figure = tester.add("figure").set("points", 10);
  auto colour = tester.add("colour")
    .set("r", 1.0)
    .set("g", 0.5)
    .set("b", 0.3);
  auto fade = tester.add("fade")
    .set("r", 0.7)
    .set("g", 0.6)
    .set("b", 0.5);

  figure.connect("default", colour, "default");
  colour.connect("default", fade, "default");

  tester.run();
  Frame *frame = tester.get_frame();
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
  FrameGraphTester tester{loader};

  auto figure = tester.add("figure").set("points", 10);
  auto colour = tester.add("colour")
    .set("r", 1.0)
    .set("g", 0.5)
    .set("b", 0.3);
  auto set = tester.add("set").set("value", 0.5);
  auto fade = tester.add("fade");

  figure.connect("default", colour, "default");
  colour.connect("default", fade, "default");
  set.connect(fade, "all");

  tester.run();
  Frame *frame = tester.get_frame();
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
  FrameGraphTester tester{loader};

  auto figure = tester.add("figure").set("points", 10);
  auto colour = tester.add("colour")
    .set("r", 1.0)
    .set("g", 0.5)
    .set("b", 0.3);
  auto setr = tester.add("set").set("value", 0.7);
  auto setg = tester.add("set").set("value", 0.6);
  auto setb = tester.add("set").set("value", 0.5);
  auto fade = tester.add("fade");

  figure.connect("default", colour, "default");
  colour.connect("default", fade, "default");
  setr.connect(fade, "r");
  setg.connect(fade, "g");
  setb.connect(fade, "b");

  tester.run();
  Frame *frame = tester.get_frame();
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
  loader.add_default_section("vector");
  return RUN_ALL_TESTS();
}
