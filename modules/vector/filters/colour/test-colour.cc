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
  FrameGraphTester tester{loader};

  auto figure = tester.add("figure").set("points", 10);
  auto colour = tester.add("colour");

  figure.connect("default", colour, "default");

  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 11 points at (0,0), black
  EXPECT_EQ(11, frame->points.size());
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
  FrameGraphTester tester{loader};

  auto figure = tester.add("figure").set("points", 10);
  auto colour = tester.add("colour")
    .set("r", 0.1)
    .set("g", 0.2)
    .set("b", 0.3);

  figure.connect("default", colour, "default");

  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(11, frame->points.size());
  bool first=true;
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(0.0, p.x);
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
    EXPECT_EQ(first?0.0:0.1, p.c.r);
    EXPECT_EQ(first?0.0:0.2, p.c.g);
    EXPECT_EQ(first?0.0:0.3, p.c.b);
    first = false;
  }
}

TEST(ColourTest, TestSpecifiedHSLColour)
{
  FrameGraphTester tester{loader};

  auto figure = tester.add("figure").set("points", 10);
  auto colour = tester.add("colour")
    .set("h", 0)
    .set("s", 1.0)
    .set("l", 0.5);
  // Note the above tests order dependency, because setting 'h' and 's' when
  // l is 0, will result in RGB black, then setting l=0.5 makes grey.

  figure.connect("default", colour, "default");

  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(11, frame->points.size());
  bool first=true;
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(0.0, p.x);
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
    EXPECT_EQ(first?0.0:1.0, p.c.r);
    EXPECT_EQ(0.0, p.c.g);
    EXPECT_EQ(0.0, p.c.b);
    first = false;
  }
}

TEST(ColourTest, TestSpecifiedHexColourWithHash)
{
  FrameGraphTester tester{loader};

  auto figure = tester.add("figure").set("points", 10);
  auto colour = tester.add("colour").set("hex", "#00c0ff");

  figure.connect("default", colour, "default");

  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(11, frame->points.size());
  bool first=true;
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(0.0, p.x);
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
    EXPECT_DOUBLE_EQ(0.0, p.c.r);
    EXPECT_NEAR(first?0.0:0.75, p.c.g, 0.01);
    EXPECT_DOUBLE_EQ(first?0.0:1.0, p.c.b);
    first = false;
  }
}

TEST(ColourTest, TestSpecifiedHexColourWithoutHash)
{
  FrameGraphTester tester{loader};

  auto figure = tester.add("figure").set("points", 10);
  auto colour = tester.add("colour").set("hex", "00c0ff");

  figure.connect("default", colour, "default");

  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(11, frame->points.size());
  bool first=true;
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(0.0, p.x);
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
    EXPECT_DOUBLE_EQ(0.0, p.c.r);
    EXPECT_NEAR(first?0.0:0.75, p.c.g, 0.01);
    EXPECT_DOUBLE_EQ(first?0.0:1.0, p.c.b);
    first = false;
  }
}

TEST(ColourTest, TestColourRGBPropertiesChanged)
{
  FrameGraphTester tester{loader};

  auto setr = tester.add("set").set("value", 0.4);
  auto setg = tester.add("set").set("value", 0.5);
  auto setb = tester.add("set").set("value", 0.6);
  auto figure = tester.add("figure").set("points", 10);
  auto colour = tester.add("colour")
    .set("r", 0.1)
    .set("g", 0.2)
    .set("b", 0.3);

  setr.connect(colour, "r");
  setg.connect(colour, "g");
  setb.connect(colour, "b");
  figure.connect("default", colour, "default");

  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(11, frame->points.size());
  bool first=true;
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(0.0, p.x);
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
    EXPECT_EQ(first?0.0:0.4, p.c.r);
    EXPECT_EQ(first?0.0:0.5, p.c.g);
    EXPECT_EQ(first?0.0:0.6, p.c.b);
    first=false;
  }
}

TEST(ColourTest, TestColourHSLPropertiesChanged)
{
  FrameGraphTester tester{loader};

  auto seth = tester.add("set").set("value", 0.33333);
  auto sets = tester.add("set").set("value", 1.0);
  auto setl = tester.add("set").set("value", 0.5);
  auto figure = tester.add("figure").set("points", 10);
  auto colour = tester.add("colour")
    .set("h", 0)
    .set("s", 0)
    .set("l", 0);

  // Note order of sets - this tests separate HSL storage
  seth.connect(colour, "h");
  sets.connect(colour, "s");
  setl.connect(colour, "l");
  figure.connect("default", colour, "default");

  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(11, frame->points.size());
  bool first=true;
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(0.0, p.x);
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
    EXPECT_NEAR(0.0, p.c.r, 0.01);
    EXPECT_NEAR(first?0.0:1.0, p.c.g, 0.01);
    EXPECT_NEAR(0.0, p.c.b, 0.01);
    first = false;
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
  loader.add_default_section("vector");
  return RUN_ALL_TESTS();
}
