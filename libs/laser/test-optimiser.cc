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

TEST(OptimiserTest, TestPassThroughIfNoOperationsConfigured)
{
  vector<Point> points;
  points.push_back(Point(0, 0));
  points.push_back(Point(1, 1));
  points.push_back(Point(2, 2));

  Optimiser optimiser;
  vector<Point> opoints = optimiser.optimise(points);
  ASSERT_EQ(3, opoints.size());
  ASSERT_EQ(Point(0,0), opoints[0]);
  EXPECT_EQ(Point(1,1), opoints[1]);
  EXPECT_EQ(Point(2,2), opoints[2]);
}

TEST(OptimiserTest, TestPassThroughIfMaxDistanceNotExceeded)
{
  vector<Point> points;
  points.push_back(Point(0, 0));
  points.push_back(Point(1, 1));
  points.push_back(Point(2, 2));

  Optimiser optimiser;
  optimiser.enable_infill(1);
  vector<Point> opoints = optimiser.optimise(points);
  ASSERT_EQ(3, opoints.size());
  ASSERT_EQ(Point(0,0), opoints[0]);
  EXPECT_EQ(Point(1,1), opoints[1]);
  EXPECT_EQ(Point(2,2), opoints[2]);
}

TEST(OptimiserTest, TestMaxDistanceInfill)
{
  vector<Point> points;
  points.push_back(Point(0, 0));
  points.push_back(Point(1, 0));

  Optimiser optimiser;
  optimiser.enable_infill(0.5);
  vector<Point> opoints = optimiser.optimise(points);
  ASSERT_EQ(3, opoints.size());
  ASSERT_EQ(Point(0,0), opoints[0]);
  EXPECT_NEAR(0.5, opoints[1].x, 0.01);
  EXPECT_EQ(0, opoints[1].y);
  EXPECT_EQ(Point(1,0), opoints[2]);
}

TEST(OptimiserTest, TestPassThroughIfMaxAngleNotExceeded)
{
  vector<Point> points;
  points.push_back(Point(0, 0));
  points.push_back(Point(1, 1));
  points.push_back(Point(1.1, 2));

  Optimiser optimiser;
  optimiser.enable_vertex_repeats(pi/4, 3);
  vector<Point> opoints = optimiser.optimise(points);
  ASSERT_EQ(3, opoints.size());
  ASSERT_EQ(Point(0,0), opoints[0]);
  EXPECT_EQ(Point(1,1), opoints[1]);
  EXPECT_EQ(Point(1.1,2), opoints[2]);
}

TEST(OptimiserTest, TestRepeatsAddedIfMaxAngleExceeded)
{
  vector<Point> points;
  points.push_back(Point(0, 0));
  points.push_back(Point(1, 1));
  points.push_back(Point(0.9, 2));

  Optimiser optimiser;
  optimiser.enable_vertex_repeats(pi/4, 3);
  vector<Point> opoints = optimiser.optimise(points);
  ASSERT_EQ(6, opoints.size());
  ASSERT_EQ(Point(0,0), opoints[0]);
  EXPECT_EQ(Point(1,1), opoints[1]);
  EXPECT_EQ(Point(1,1), opoints[2]);
  EXPECT_EQ(Point(1,1), opoints[3]);
  EXPECT_EQ(Point(1,1), opoints[4]);
  EXPECT_EQ(Point(0.9,2), opoints[5]);
}

TEST(OptimiserTest, TestRepeatsAddedForBlanking)
{
  vector<Point> points;
  points.push_back(Point(0, 0));
  points.push_back(Point(1, 1, Colour::white));

  Optimiser optimiser;
  optimiser.enable_blanking_repeats(3);
  vector<Point> opoints = optimiser.optimise(points);
  ASSERT_EQ(8, opoints.size());
  ASSERT_EQ(Point(0,0), opoints[0]);
  ASSERT_TRUE(opoints[0].is_blanked());
  EXPECT_EQ(Point(0,0), opoints[1]);
  ASSERT_TRUE(opoints[1].is_blanked());
  EXPECT_EQ(Point(0,0), opoints[2]);
  ASSERT_TRUE(opoints[2].is_blanked());
  EXPECT_EQ(Point(0,0), opoints[3]);
  ASSERT_TRUE(opoints[3].is_blanked());
  EXPECT_EQ(Point(1,1), opoints[4]);
  EXPECT_EQ(Colour::white, opoints[4].c);
  EXPECT_EQ(Point(1,1), opoints[5]);
  EXPECT_EQ(Colour::white, opoints[5].c);
  EXPECT_EQ(Point(1,1), opoints[6]);
  EXPECT_EQ(Colour::white, opoints[6].c);
  EXPECT_EQ(Point(1,1), opoints[7]);
  EXPECT_EQ(Colour::white, opoints[7].c);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
