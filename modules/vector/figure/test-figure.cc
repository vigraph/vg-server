//==========================================================================
// ViGraph dataflow module: vector/figure/test-figure.cc
//
// Tests for figure pattern generator
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include "../../module-test.h"
#include "vg-waveform.h"
#include <cmath>

class FigureTest: public GraphTester
{
public:
  FigureTest()
  {
    loader.load("./vg-module-vector-figure.so");
  }
};

const auto sample_rate = 1;

TEST_F(FigureTest, TestNull)
{
  auto& fig = add("vector/figure");
  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  fig.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(101, frame.points.size());
  for(auto i=0u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    EXPECT_EQ(0, p.x);
    EXPECT_EQ(0, p.y);
    EXPECT_EQ(0, p.z);
    EXPECT_EQ(i ? Colour::white : Colour::black, p.c);
  }
}

TEST_F(FigureTest, TestSpecifiedPoints)
{
  auto& fig = add("vector/figure")
              .set("points", 33.0);
  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  fig.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(34, frame.points.size());
}

TEST_F(FigureTest, TestClosedAddsExtraPoint)
{
  auto& fig = add("vector/figure")
              .set("closed", 1.0);
  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  fig.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(102, frame.points.size());
}

TEST_F(FigureTest, TestFlatline)
{
  auto& fig = add("vector/figure")
              .set("x-wave", Waveform::Type::saw);

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  fig.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];

  // Should be 100 points along horizontal line -0.5 .. 0.5, plus blank
  ASSERT_EQ(101, frame.points.size());
  EXPECT_EQ(Point(-0.5,0,0), frame.points[0]);
  EXPECT_TRUE(frame.points[0].is_blanked());

  for(auto i=1u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    EXPECT_NEAR(-0.5 + (i-1)/100.0, p.x, 1e-6) << i;
    EXPECT_EQ(0.0, p.y) << i;
    EXPECT_EQ(0.0, p.z) << i;
    EXPECT_TRUE(p.is_lit());
  }
}

TEST_F(FigureTest, TestSawXYZ)
{
  auto& fig = add("vector/figure")
              .set("x-wave", Waveform::Type::saw)
              .set("y-wave", Waveform::Type::saw)
              .set("z-wave", Waveform::Type::saw);

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  fig.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];

  // Should be 100 points along horizontal line -0.5 .. 0.5 on all 3 axes,
  // plus blank
  ASSERT_EQ(101, frame.points.size());
  EXPECT_EQ(Point(-0.5,-0.5,-0.5), frame.points[0]);
  EXPECT_TRUE(frame.points[0].is_blanked());

  for(auto i=1u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    EXPECT_NEAR(-0.5 + (i-1)/100.0, p.x, 1e-6) << i;
    EXPECT_NEAR(-0.5 + (i-1)/100.0, p.y, 1e-6) << i;
    EXPECT_NEAR(-0.5 + (i-1)/100.0, p.z, 1e-6) << i;
    EXPECT_TRUE(p.is_lit());
  }
}

TEST_F(FigureTest, TestSawXYZFrequencies)
{
  auto& fig = add("vector/figure")
              .set("x-wave", Waveform::Type::saw)
              .set("x-freq", 2.0)
              .set("y-wave", Waveform::Type::saw)
              .set("y-freq", 3.0)
              .set("z-wave", Waveform::Type::saw)
              .set("z-freq", 4.0);

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  fig.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];

  ASSERT_EQ(101, frame.points.size());
  EXPECT_EQ(Point(-0.5,-0.5,-0.5), frame.points[0]);
  EXPECT_TRUE(frame.points[0].is_blanked());

  for(auto i=1u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    double d;
    EXPECT_NEAR(-0.5 + modf(2.0*(i-1)/100.0, &d), p.x, 1e-6) << i;
    EXPECT_NEAR(-0.5 + modf(3.0*(i-1)/100.0, &d), p.y, 1e-6) << i;
    EXPECT_NEAR(-0.5 + modf(4.0*(i-1)/100.0, &d), p.z, 1e-6) << i;
    EXPECT_TRUE(p.is_lit());
  }
}

TEST_F(FigureTest, TestSawXYZPhases)
{
  auto& fig = add("vector/figure")
              .set("x-wave", Waveform::Type::saw)
              .set("x-phase", 0.5)
              .set("y-wave", Waveform::Type::saw)
              .set("y-phase", -0.6)
              .set("z-wave", Waveform::Type::saw)
              .set("z-phase", 1.0);

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  fig.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];

  ASSERT_EQ(101, frame.points.size());
  EXPECT_EQ(0, frame.points[0].x);
  EXPECT_NEAR(-0.1, frame.points[0].y, 1e-6);
  EXPECT_NEAR(-0.5, frame.points[0].z, 1e-6);
  EXPECT_TRUE(frame.points[0].is_blanked());

  for(auto i=1u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    double d;
    EXPECT_NEAR(-0.5 + modf((i-1)/100.0+0.5, &d),     p.x, 1e-6) << i;
    EXPECT_NEAR(-0.5 + modf((i-1)/100.0+1.0-0.6, &d), p.y, 1e-6) << i;
    EXPECT_NEAR(-0.5 + (i-1)/100.0,                   p.z, 1e-6) << i;
    EXPECT_TRUE(p.is_lit());
  }
}

TEST_F(FigureTest, TestSquareXYZ)
{
  auto& fig = add("vector/figure")
              .set("x-wave", Waveform::Type::square)
              .set("y-wave", Waveform::Type::square)
              .set("z-wave", Waveform::Type::square);

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  fig.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];

  ASSERT_EQ(101, frame.points.size());
  EXPECT_EQ(Point(0.5,0.5,0.5), frame.points[0]);
  EXPECT_TRUE(frame.points[0].is_blanked());

  for(auto i=1u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    EXPECT_DOUBLE_EQ((i>50)?-0.5:0.5, p.x) << i;
    EXPECT_DOUBLE_EQ((i>50)?-0.5:0.5, p.y) << i;
    EXPECT_DOUBLE_EQ((i>50)?-0.5:0.5, p.z) << i;
    EXPECT_TRUE(p.is_lit());
  }
}


TEST_F(FigureTest, TestTriangleXYZ)
{
  auto& fig = add("vector/figure")
              .set("x-wave", Waveform::Type::triangle)
              .set("y-wave", Waveform::Type::triangle)
              .set("z-wave", Waveform::Type::triangle);

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  fig.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];

  ASSERT_EQ(101, frame.points.size());
  EXPECT_EQ(Point(0,0,0), frame.points[0]);
  EXPECT_TRUE(frame.points[0].is_blanked());

  for(auto i=1u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    const auto v = (i<=25) ? (i-1)/50
                : ((i<=75) ? 0.5-(i-26)/50
                           : -0.5+(i-76)/50);
    EXPECT_NEAR(v, p.x, 1e6) << i;
    EXPECT_NEAR(v, p.y, 1e6) << i;
    EXPECT_NEAR(v, p.y, 1e6) << i;
    EXPECT_TRUE(p.is_lit());
  }
}

TEST_F(FigureTest, TestSinXYZ)
{
  auto& fig = add("vector/figure")
              .set("x-wave", Waveform::Type::sin)
              .set("y-wave", Waveform::Type::sin)
              .set("z-wave", Waveform::Type::sin);

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  fig.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];

  ASSERT_EQ(101, frame.points.size());
  EXPECT_EQ(Point(0,0,0), frame.points[0]);
  EXPECT_TRUE(frame.points[0].is_blanked());

  for(auto i=1u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    const auto v = 0.5*sin((i-1)/100.0*2*pi);
    EXPECT_NEAR(v, p.x, 1e6) << i;
    EXPECT_NEAR(v, p.y, 1e6) << i;
    EXPECT_NEAR(v, p.y, 1e6) << i;
    EXPECT_TRUE(p.is_lit());
  }
}

TEST_F(FigureTest, TestRandomXYZ)
{
  auto& fig = add("vector/figure")
              .set("x-wave", Waveform::Type::random)
              .set("y-wave", Waveform::Type::random)
              .set("z-wave", Waveform::Type::random);

  auto frames = vector<Frame>{};
  auto& snk = add_sink(frames, sample_rate);
  fig.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];

  ASSERT_EQ(101, frame.points.size());
  EXPECT_TRUE(frame.points[0].is_blanked());

  // 100 points of randomness -0.5 .. 0.5 on 3 axes, all different
  for(auto i=1u; i<frame.points.size(); i++)
  {
    const auto& p = frame.points[i];
    EXPECT_GE(0.5, p.x);
    EXPECT_LE(-0.5, p.x);
    EXPECT_GE(0.5, p.y);
    EXPECT_LE(-0.5, p.y);
    EXPECT_GE(0.5, p.z);
    EXPECT_LE(-0.5, p.z);
    EXPECT_NE(p.x, p.y);  // Vanishingly unlikely and checks for all (0,0,0)
    EXPECT_NE(p.x, p.z);
    EXPECT_NE(p.y, p.z);
    EXPECT_TRUE(p.is_lit());

    // Ensure all different!
    for(auto j=1u; j<i; j++)
    {
      const auto& q = frame.points[j];
      EXPECT_NE(p, q) << "Same " << i << " and " << j;
    }
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
  return RUN_ALL_TESTS();
}
