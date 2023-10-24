//==========================================================================
// ViGraph laser graphics library: test-optimiser.cc
//
// Tests for laser frame optimiser
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-laser.h"
#include <gtest/gtest.h>
#include <sstream>

namespace {

using namespace ViGraph;
using namespace ViGraph::Geometry;
using namespace ViGraph::Laser;

TEST(OptimiserTest, TestInfillPassThroughIfMaxDistanceNotExceeded)
{
  vector<Point> points;
  points.push_back(Point(0, 0));
  points.push_back(Point(1, 0, Colour::white));
  points.push_back(Point(1, 1));

  Optimiser optimiser;
  vector<Point> opoints = optimiser.infill_lines(points, 2, 2);
  ASSERT_EQ(3, opoints.size());
  ASSERT_EQ(Point(0,0), opoints[0]);
  EXPECT_EQ(Point(1,0), opoints[1]);
  EXPECT_EQ(Point(1,1), opoints[2]);
}

TEST(OptimiserTest, TestInfillAddsBlanks)
{
  vector<Point> points;
  points.push_back(Point(0, 0));
  points.push_back(Point(1, 0));

  Optimiser optimiser;
  vector<Point> opoints = optimiser.infill_lines(points, 0, 0.5);
  ASSERT_EQ(4, opoints.size());
  ASSERT_EQ(Point(0,0), opoints[0]);
  EXPECT_NEAR(0.5, opoints[1].x, 0.01);
  EXPECT_EQ(0, opoints[1].y);
  EXPECT_TRUE(opoints[1].is_blanked());
  EXPECT_EQ(Point(1,0), opoints[2]);
  EXPECT_NEAR(0.5, opoints[3].x, 0.01);  // flyback
  EXPECT_EQ(0, opoints[3].y);
  EXPECT_TRUE(opoints[3].is_blanked());
}

TEST(OptimiserTest, TestInfillAddsPoints)
{
  vector<Point> points;
  points.push_back(Point(0, 0));
  points.push_back(Point(1, 0, Colour::white));

  Optimiser optimiser;
  vector<Point> opoints = optimiser.infill_lines(points, 0.5, 0);
  ASSERT_EQ(3, opoints.size());
  ASSERT_EQ(Point(0,0), opoints[0]);
  EXPECT_NEAR(0.5, opoints[1].x, 0.01);
  EXPECT_EQ(0, opoints[1].y);
  EXPECT_TRUE(opoints[1].is_lit());
  EXPECT_EQ(Point(1,0), opoints[2]);
}

TEST(OptimiserTest, TestVertexPassThroughIfMaxAngleNotExceeded)
{
  vector<Point> points;
  points.push_back(Point(0, 0));
  points.push_back(Point(1, 1));
  points.push_back(Point(1.1, 2));

  Optimiser optimiser;
  vector<Point> opoints = optimiser.add_vertex_repeats(points, pi/4, 3);
  ASSERT_EQ(3, opoints.size());
  ASSERT_EQ(Point(0,0), opoints[0]);
  EXPECT_EQ(Point(1,1), opoints[1]);
  EXPECT_EQ(Point(1.1,2), opoints[2]);
}

TEST(OptimiserTest, TestVertexRepeatsAddedIfMaxAngleExceededLeft)
{
  vector<Point> points;
  points.push_back(Point(0, 0));
  points.push_back(Point(1, 1));
  points.push_back(Point(0.9, 2));

  Optimiser optimiser;
  vector<Point> opoints = optimiser.add_vertex_repeats(points, pi/4, 3);
  ASSERT_EQ(6, opoints.size());
  ASSERT_EQ(Point(0,0), opoints[0]);
  EXPECT_EQ(Point(1,1), opoints[1]);
  EXPECT_EQ(Point(1,1), opoints[2]);
  EXPECT_EQ(Point(1,1), opoints[3]);
  EXPECT_EQ(Point(1,1), opoints[4]);
  EXPECT_EQ(Point(0.9,2), opoints[5]);
}

TEST(OptimiserTest, TestVertexRepeatsAddedIfMaxAngleExceededRight)
{
  vector<Point> points;
  points.push_back(Point(1, 1));
  points.push_back(Point(1, 0));
  points.push_back(Point(0, 0));

  Optimiser optimiser;
  vector<Point> opoints = optimiser.add_vertex_repeats(points, pi/4, 3);
  ASSERT_EQ(6, opoints.size());
  ASSERT_EQ(Point(1,1), opoints[0]);
  EXPECT_EQ(Point(1,0), opoints[1]);
  EXPECT_EQ(Point(1,0), opoints[2]);
  EXPECT_EQ(Point(1,0), opoints[3]);
  EXPECT_EQ(Point(1,0), opoints[4]);
  EXPECT_EQ(Point(0,0), opoints[5]);
}

TEST(OptimiserTest, TestBlankingAnchorsAdded)
{
  vector<Point> points;
  points.push_back(Point(0, 0));
  points.push_back(Point(1, 0, Colour::white));
  points.push_back(Point(2, 0));
  points.push_back(Point(3, 0, Colour::white));

  Optimiser optimiser;
  vector<Point> opoints = optimiser.add_blanking_anchors(points, 1, 2);

  ASSERT_EQ(10, opoints.size());  // 1 at start, 2 added at 1, 1 at 2, 2 at end
  int i=0;
  // added leading at start
  EXPECT_EQ(Point(0,0), opoints[i]);
  EXPECT_TRUE(opoints[i++].is_blanked());
  // original
  EXPECT_EQ(Point(0,0), opoints[i]);
  EXPECT_TRUE(opoints[i++].is_blanked());
  // added trailing
  EXPECT_EQ(Point(1,0), opoints[i]);
  EXPECT_EQ(Colour::white, opoints[i++].c);
  EXPECT_EQ(Point(1,0), opoints[i]);
  EXPECT_EQ(Colour::white, opoints[i++].c);
  // original
  EXPECT_EQ(Point(1,0), opoints[i]);
  EXPECT_EQ(Colour::white, opoints[i++].c);
  // added leading
  EXPECT_EQ(Point(2,0), opoints[i]);
  EXPECT_TRUE(opoints[i++].is_blanked());
  // original
  EXPECT_EQ(Point(2,0), opoints[i]);
  EXPECT_TRUE(opoints[i++].is_blanked());
  // original
  EXPECT_EQ(Point(3,0), opoints[i]);
  EXPECT_EQ(Colour::white, opoints[i++].c);
  // added trailing at end
  EXPECT_EQ(Point(3,0), opoints[i]);
  EXPECT_EQ(Colour::white, opoints[i++].c);
  EXPECT_EQ(Point(3,0), opoints[i]);
  EXPECT_EQ(Colour::white, opoints[i++].c);

  EXPECT_EQ(10, i);
}

TEST(OptimiserTest, TestReordering)
{
  vector<Point> points;
  points.push_back(Point(0, 0));
  points.push_back(Point(1, 1, Colour::white));

  points.push_back(Point(3, 3));
  points.push_back(Point(4, 4, Colour::white));

  points.push_back(Point(1, 1));
  points.push_back(Point(2, 2, Colour::white));

  Optimiser optimiser;
  vector<Point> opoints = optimiser.reorder_segments(points);
  ASSERT_EQ(6, opoints.size());

  EXPECT_EQ(0, opoints[0].x);
  EXPECT_EQ(1, opoints[1].x);
  EXPECT_EQ(1, opoints[2].x);
  EXPECT_EQ(2, opoints[3].x);
  EXPECT_EQ(3, opoints[4].x);
  EXPECT_EQ(4, opoints[5].x);
}

TEST(OptimiserTest, TestReorderingWithLastPointBlank)
{
  vector<Point> points;
  points.push_back(Point(0, 0));
  points.push_back(Point(1, 1, Colour::white));
  points.push_back(Point(2, 2, Colour::white));
  points.push_back(Point(3, 3, Colour::white));
  points.push_back(Point(4, 4));

  Optimiser optimiser;
  vector<Point> opoints = optimiser.reorder_segments(points);
  ASSERT_EQ(5, opoints.size());

  EXPECT_EQ(0, opoints[0].x);
  EXPECT_EQ(1, opoints[1].x);
  EXPECT_EQ(2, opoints[2].x);
  EXPECT_EQ(3, opoints[3].x);
  EXPECT_EQ(4, opoints[4].x);
}

TEST(OptimiserTest, TestStripBlanks)
{
  vector<Point> points;
  for(int i=0; i<10; i++)
    points.push_back(Point(0, 0));
  for(int i=0; i<10; i++)
    points.push_back(Point(0, 0, Colour::white));
  for(int i=0; i<5; i++)
    points.push_back(Point(0, 0));
  points.push_back(Point(0, 0, Colour::white));
  for(int i=0; i<10; i++)
    points.push_back(Point(0, 0));

  Optimiser optimiser;
  vector<Point> opoints = optimiser.strip_blank_runs(points, 5);
  ASSERT_EQ(26, opoints.size());

  for(int i=0; i<5; i++)
    EXPECT_TRUE(opoints[i].is_blanked());
  for(int i=5; i<15; i++)
    EXPECT_TRUE(opoints[i].is_lit());
  for(int i=15; i<20; i++)
    EXPECT_TRUE(opoints[i].is_blanked());
  EXPECT_TRUE(opoints[20].is_lit());
  for(int i=21; i<26; i++)
    EXPECT_TRUE(opoints[i].is_blanked());
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
