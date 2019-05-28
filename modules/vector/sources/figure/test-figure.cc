//==========================================================================
// ViGraph dataflow module: vector/sources/figure/test-figure.cc
//
// Tests for <figure> source
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module-test.h"
#include <cmath>

ModuleLoader loader;

TEST(FigureTest, TestNull)
{
  FrameGraphTester tester{loader};

  tester.add("figure");
  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 101 points at (0,0), white except first
  EXPECT_EQ(101, frame->points.size());
  EXPECT_EQ(Point(0,0,0), frame->points[0]);
  EXPECT_TRUE(frame->points[0].is_blanked());

  for(auto i=1u; i<frame->points.size(); i++)
  {
    const auto& p=frame->points[i];
    EXPECT_EQ(0.0, p.x);
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
    EXPECT_EQ(1.0, p.c.r);
    EXPECT_EQ(1.0, p.c.g);
    EXPECT_EQ(1.0, p.c.b);
  }
}

TEST(FigureTest, TestSpecifiedPoints)
{
  FrameGraphTester tester{loader};

  tester.add("figure").set("points", 33);
  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(34, frame->points.size());
}

TEST(FigureTest, TestClosedAddsExtraPoint)
{
  FrameGraphTester tester{loader};

  tester.add("figure").set("closed", true);
  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  ASSERT_EQ(102, frame->points.size());
  EXPECT_EQ(frame->points[1], frame->points[101]);
}

TEST(FigureTest, TestNullWithPosOffset)
{
  FrameGraphTester tester{loader};

  tester.add("figure")
    .set("x.pos", 0.2)
    .set("y.pos", 0.3);
  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(101, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(0.2, p.x);
    EXPECT_EQ(0.3, p.y);
  }
}

TEST(FigureTest, TestFlatline)
{
  FrameGraphTester tester{loader};

  tester.add("figure").set("x.wave", "saw");
  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 100 points along horizontal line -0.5 .. 0.5, plus blank
  EXPECT_EQ(101, frame->points.size());
  EXPECT_EQ(Point(-0.5,0,0), frame->points[0]);
  EXPECT_TRUE(frame->points[0].is_blanked());

  for(auto i=1u; i<frame->points.size(); i++)
  {
    const auto& p=frame->points[i];
    EXPECT_DOUBLE_EQ(-0.5 + (i-1)/100.0, p.x) << i;
    EXPECT_EQ(0.0, p.y) << i;
    EXPECT_TRUE(p.is_lit());
  }
}

TEST(FigureTest, TestSawXY)
{
  FrameGraphTester tester{loader};

  tester.add("figure")
    .set("x.wave", "saw")
    .set("y.wave", "saw");
  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 100 points along line -0.5 .. 0.5 on both axes
  EXPECT_EQ(101, frame->points.size());
  EXPECT_EQ(Point(-0.5,-0.5,0), frame->points[0]);
  EXPECT_TRUE(frame->points[0].is_blanked());

  for(auto i=1u; i<frame->points.size(); i++)
  {
    const auto& p=frame->points[i];
    EXPECT_DOUBLE_EQ(-0.5 + (i-1)/100.0, p.x) << i;
    EXPECT_DOUBLE_EQ(-0.5 + (i-1)/100.0, p.y) << i;
    EXPECT_TRUE(p.is_lit());
  }
}

TEST(FigureTest, TestSawXYScaled)
{
  FrameGraphTester tester{loader};

  tester.add("figure")
    .set("x.wave", "saw")
    .set("x.scale", 2)
    .set("y.wave", "saw")
    .set("y.scale", 4);
  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 100 points along line (-1.0, -2.0) .. (1.0, 2.0)
  EXPECT_EQ(101, frame->points.size());
  EXPECT_EQ(Point(-1,-2,0), frame->points[0]);
  EXPECT_TRUE(frame->points[0].is_blanked());

  for(auto i=1u; i<frame->points.size(); i++)
  {
    const auto& p=frame->points[i];
    EXPECT_DOUBLE_EQ(-1.0 + 2.0*(i-1)/100.0, p.x) << i;
    EXPECT_DOUBLE_EQ(-2.0 + 4.0*(i-1)/100.0, p.y) << i;
    EXPECT_TRUE(p.is_lit());
  }
}

TEST(FigureTest, TestSawXYFrequencies)
{
  FrameGraphTester tester{loader};

  tester.add("figure")
    .set("x.wave", "saw")
    .set("x.freq", 2.0)
    .set("y.wave", "saw")
    .set("y.freq", 3.0);
  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(101, frame->points.size());
  EXPECT_EQ(Point(-0.5,-0.5,0), frame->points[0]);
  EXPECT_TRUE(frame->points[0].is_blanked());

  for(auto i=1u; i<frame->points.size(); i++)
  {
    const auto& p=frame->points[i];
    double d;
    EXPECT_DOUBLE_EQ(-0.5 + modf(2.0*(i-1)/100.0, &d), p.x) << i;
    EXPECT_DOUBLE_EQ(-0.5 + modf(3.0*(i-1)/100.0, &d), p.y) << i;
    EXPECT_TRUE(p.is_lit());
  }
}

TEST(FigureTest, TestSawXYPhases)
{
  FrameGraphTester tester{loader};

  tester.add("figure")
    .set("x.wave", "saw")
    .set("x.phase", 0.5)
    .set("y.wave", "saw")
    .set("y.phase", -0.6);
  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(101, frame->points.size());
  EXPECT_EQ(0, frame->points[0].x);
  EXPECT_NEAR(-0.1, frame->points[0].y, 0.00001);
  EXPECT_TRUE(frame->points[0].is_blanked());

  for(auto i=1u; i<frame->points.size(); i++)
  {
    const auto& p=frame->points[i];
    double d;
    // Note:  Within 1 in 100,000 is good enough for 16 bit
    EXPECT_NEAR(-0.5 + modf((i-1)/100.0+0.5, &d),     p.x, 0.00001) << i;
    EXPECT_NEAR(-0.5 + modf((i-1)/100.0+1.0-0.6, &d), p.y, 0.00001) << i;
    EXPECT_TRUE(p.is_lit());
  }
}

TEST(FigureTest, TestSquareXY)
{
  FrameGraphTester tester{loader};

  tester.add("figure")
    .set("x.wave", "square")
    .set("y.wave", "square");
  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(101, frame->points.size());
  EXPECT_EQ(Point(-0.5,-0.5,0), frame->points[0]);
  EXPECT_TRUE(frame->points[0].is_blanked());

  for(auto i=1u; i<frame->points.size(); i++)
  {
    const auto& p=frame->points[i];
    EXPECT_DOUBLE_EQ((i-1>=50)?0.5:-0.5, p.x) << i;
    EXPECT_DOUBLE_EQ((i-1>=50)?0.5:-0.5, p.y) << i;
    EXPECT_TRUE(p.is_lit());
  }
}

TEST(FigureTest, TestTriangleXY)
{
  FrameGraphTester tester{loader};

  tester.add("figure")
    .set("x.wave", "triangle")
    .set("y.wave", "triangle");
  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(101, frame->points.size());
  EXPECT_EQ(Point(-0.5,-0.5,0), frame->points[0]);
  EXPECT_TRUE(frame->points[0].is_blanked());

  // 100 points of -0.5..0.5 and back on both axes
  for(auto i=1u; i<frame->points.size(); i++)
  {
    const auto& p=frame->points[i];
    EXPECT_NEAR((i-1<50)?((i-1)/50.0-0.5):((101-i)/50.0-0.5), p.x, 0.00001)<< i;
    EXPECT_NEAR((i-1<50)?((i-1)/50.0-0.5):((101-i)/50.0-0.5), p.y, 0.00001)<< i;
    EXPECT_TRUE(p.is_lit());
  }
}

TEST(FigureTest, TestSinXY)
{
  FrameGraphTester tester{loader};

  tester.add("figure")
    .set("x.wave", "sin")
    .set("y.wave", "sin");
  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(101, frame->points.size());
  EXPECT_EQ(Point(0,0,0), frame->points[0]);
  EXPECT_TRUE(frame->points[0].is_blanked());

  // 100 points of sin waves -0.5 .. 0.5 on both axes
  for(auto i=1u; i<frame->points.size(); i++)
  {
    const auto& p=frame->points[i];
    EXPECT_NEAR(0.5*sin((i-1)/100.0*2*pi), p.x, 0.00001) << i;
    EXPECT_NEAR(0.5*sin((i-1)/100.0*2*pi), p.y, 0.00001) << i;
    EXPECT_TRUE(p.is_lit());
  }
}

TEST(FigureTest, TestRandomXY)
{
  FrameGraphTester tester{loader};

  tester.add("figure")
    .set("x.wave", "random")
    .set("y.wave", "random");
  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(101, frame->points.size());
  EXPECT_TRUE(frame->points[0].is_blanked());

  // 100 points of randomness -0.5 .. 0.5 on both axes
  for(auto i=1u; i<frame->points.size(); i++)
  {
    const auto& p=frame->points[i];
    EXPECT_GE(0.5, p.x);
    EXPECT_LE(-0.5, p.x);
    EXPECT_GE(0.5, p.y);
    EXPECT_LE(-0.5, p.y);
    EXPECT_NE(p.x, p.y);  // Vanishingly unlikely and checks for all (0,0)
    EXPECT_TRUE(p.is_lit());
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
  loader.load("./vg-module-vector-source-figure.so");
  loader.add_default_section("vector");
  return RUN_ALL_TESTS();
}
