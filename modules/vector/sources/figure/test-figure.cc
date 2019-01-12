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
  const string& xml = R"(
    <graph>
      <figure/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
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
  const string& xml = R"(
    <graph>
      <figure points='33'/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);
  EXPECT_EQ(34, frame->points.size());
}

TEST(FigureTest, TestClosedAddsExtraPoint)
{
  const string& xml = R"(
    <graph>
      <figure closed='yes'/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);
  ASSERT_EQ(102, frame->points.size());
  EXPECT_EQ(frame->points[1], frame->points[101]);
}

TEST(FigureTest, TestNullWithPosOffset)
{
  const string& xml = R"(
    <graph>
      <figure>
        <x pos='0.2'/>
        <y pos='0.3'/>
      </figure>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
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
  const string& xml = R"(
    <graph>
      <figure>
        <x wave='saw'/>
      </figure>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
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
  }
}

TEST(FigureTest, TestSawXY)
{
  const string& xml = R"(
    <graph>
      <figure>
        <x wave='saw'/>
        <y wave='saw'/>
      </figure>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
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
  }
}

TEST(FigureTest, TestSawXYScaled)
{
  const string& xml = R"(
    <graph>
      <figure>
        <x wave='saw' scale='2'/>
        <y wave='saw' scale='4'/>
      </figure>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
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
  }
}

TEST(FigureTest, TestSawXYFrequencies)
{
  const string& xml = R"(
    <graph>
      <figure>
        <x wave='saw' freq='2.0'/>
        <y wave='saw' freq='3.0'/>
      </figure>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
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
  }
}

TEST(FigureTest, TestSawXYPhases)
{
  const string& xml = R"(
    <graph>
      <figure>
        <x wave='saw' phase='0.5'/>
        <y wave='saw' phase='-0.6'/>
      </figure>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
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
  }
}

TEST(FigureTest, TestSquareXY)
{
  const string& xml = R"(
    <graph>
      <figure>
        <x wave='square'/>
        <y wave='square'/>
      </figure>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  EXPECT_EQ(101, frame->points.size());
  EXPECT_EQ(Point(-0.5,-0.5,0), frame->points[0]);
  EXPECT_TRUE(frame->points[0].is_blanked());

  for(auto i=1u; i<frame->points.size(); i++)
  {
    const auto& p=frame->points[i];
    EXPECT_DOUBLE_EQ((i-1>=50)?0.5:-0.5, p.x) << i;
    EXPECT_DOUBLE_EQ((i-1>=50)?0.5:-0.5, p.y) << i;
  }
}

TEST(FigureTest, TestTriangleXY)
{
  const string& xml = R"(
    <graph>
      <figure>
        <x wave='triangle'/>
        <y wave='triangle'/>
      </figure>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
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
  }
}

TEST(FigureTest, TestSinXY)
{
  const string& xml = R"(
    <graph>
      <figure>
        <x wave='sin'/>
        <y wave='sin'/>
      </figure>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
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
  }
}

TEST(FigureTest, TestRandomXY)
{
  const string& xml = R"(
    <graph>
      <figure>
        <x wave='random'/>
        <y wave='random'/>
      </figure>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
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
  return RUN_ALL_TESTS();
}
