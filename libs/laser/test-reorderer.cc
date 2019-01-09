//==========================================================================
// ViGraph laser graphics library: test-reorderer.cc
//
// Tests for laser frame reordering
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-laser.h"
#include <gtest/gtest.h>
#include <sstream>

namespace {

using namespace ViGraph;
using namespace ViGraph::Geometry;
using namespace ViGraph::Laser;

TEST(ReordererTest, TestReordering)
{
  vector<Point> points;
  points.push_back(Point(0, 0));
  points.push_back(Point(1, 1, Colour::white));

  points.push_back(Point(3, 3));
  points.push_back(Point(4, 4, Colour::white));

  points.push_back(Point(1, 1));
  points.push_back(Point(2, 2, Colour::white));

  Reorderer reorderer;
  vector<Point> opoints = reorderer.reorder(points);
  ASSERT_EQ(6, opoints.size());

  EXPECT_EQ(0, opoints[0].x);
  EXPECT_EQ(1, opoints[1].x);
  EXPECT_EQ(1, opoints[2].x);
  EXPECT_EQ(2, opoints[3].x);
  EXPECT_EQ(3, opoints[4].x);
  EXPECT_EQ(4, opoints[5].x);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
