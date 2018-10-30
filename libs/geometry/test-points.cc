//==========================================================================
// ViGraph vector graphics: test-points.cc
//
// Tests for points
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-geometry.h"
#include <gtest/gtest.h>
#include <sstream>

namespace {

using namespace std;
using namespace ViGraph;
using namespace ViGraph::Geometry;

TEST(PointsTest, Test2DBlankedPointConstructionIsBlank)
{
  Point p(1,2);
  EXPECT_EQ(1, p.x);
  EXPECT_EQ(2, p.y);
  EXPECT_TRUE(p.is_blanked());
  EXPECT_FALSE(p.is_lit());
  EXPECT_EQ(p.c, Colour::black);
}

TEST(PointsTest, Test2DLitPointConstructionIsLit)
{
  Point p(1,2, Colour::red);
  EXPECT_EQ(1, p.x);
  EXPECT_EQ(2, p.y);
  EXPECT_FALSE(p.is_blanked());
  EXPECT_TRUE(p.is_lit());
  EXPECT_EQ(p.c, Colour::red);
}

TEST(PointsTest, TestPointBlankingBlanks)
{
  Point p(1,2, Colour::red);
  p.blank();
  EXPECT_TRUE(p.is_blanked());
  EXPECT_EQ(p.c, Colour::black);
}

TEST(PointsTest, Test3DBlankedPointConstructionIsBlank)
{
  Point p(1,2,3);
  EXPECT_EQ(1, p.x);
  EXPECT_EQ(2, p.y);
  EXPECT_EQ(3, p.z);
  EXPECT_TRUE(p.is_blanked());
  EXPECT_FALSE(p.is_lit());
  EXPECT_EQ(p.c, Colour::black);
}

TEST(PointsTest, Test3DLitPointConstructionIsLit)
{
  Point p(1,2,3, Colour::red);
  EXPECT_EQ(1, p.x);
  EXPECT_EQ(2, p.y);
  EXPECT_EQ(3, p.z);
  EXPECT_FALSE(p.is_blanked());
  EXPECT_TRUE(p.is_lit());
  EXPECT_EQ(p.c, Colour::red);
}

TEST(PointsTest, TestPointDistance)
{
  Point p1(0,0,0);
  Point p2(1,2,2);
  EXPECT_EQ(0, p1.distance_to(p1));
  EXPECT_EQ(3, p1.distance_to(p2));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
