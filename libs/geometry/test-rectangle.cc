//==========================================================================
// ViGraph vector graphics: test-rectangle.cc
//
// Tests for Rectangles
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-geometry.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
using namespace ViGraph::Geometry;

TEST(RectangleTest, TestRectangleIsNull)
{
  Point p0(0,0,0);
  Point p1(1,2,-3);
  Rectangle r1(p0, p0);
  EXPECT_TRUE(r1.is_null());
  EXPECT_TRUE(!r1);

  Rectangle r2(p0, p1);
  EXPECT_FALSE(r2.is_null());
  EXPECT_FALSE(!r2);
}

TEST(RectangleTest, TestRectangleEquality)
{
  Point p0(0,0,0);
  Point p1(1,2,3);
  Rectangle r1(p0, p1);
  Rectangle r2(p0, p1);
  EXPECT_EQ(r1, r2);
  EXPECT_FALSE(r1 != r2);

  Point p2(4,5,6);
  Rectangle r3(p0, p2);
  EXPECT_NE(r1, r3);
  EXPECT_FALSE(r1 == r3);
}

TEST(RectangleTest, TestRectangleNormalises)
{
  Point p0(0,0,0);
  Point p1(1,2,-3);
  Rectangle r(p0, p1);
  EXPECT_EQ(Point(0,0,-3), r.p0);
  EXPECT_EQ(Point(1,2, 0), r.p1);
}

TEST(RectangleTest, TestRectangleSize)
{
  Point p0(0,0,0);
  Point p1(1,2,-3);
  Rectangle r(p0, p1);
  Vector s = r.size();
  EXPECT_EQ(1, s.x);
  EXPECT_EQ(2, s.y);
  EXPECT_EQ(3, s.z);
}

TEST(RectangleTest, TestRectangleContainsPoint)
{
  Point p0(0,0,0);
  Point p1(1,2,-3);
  Rectangle r(p0, p1);

  EXPECT_TRUE(r.contains(p0));
  EXPECT_TRUE(r.contains(p1));
  EXPECT_FALSE(r.contains(Point(5,5,5)));
  EXPECT_FALSE(r.contains(Point(0,0,1)));
  EXPECT_TRUE(r.contains(Point(0.5, 1, -1)));
}

TEST(RectangleTest, TestOverlapDisjoint)
{
  Point p0(0,0,0);
  Point p1(1,1,1);
  Rectangle r1(p0, p1);

  Point p2(2,2,2);
  Point p3(3,3,3);
  Rectangle r2(p2, p3);

  EXPECT_FALSE(r1.overlaps(r2));
}

TEST(RectangleTest, TestOverlapTouching)
{
  Point p0(0,0,0);
  Point p1(1,1,1);
  Rectangle r1(p0, p1);

  Point p2(1,1,1);
  Point p3(2,2,2);
  Rectangle r2(p2, p3);

  EXPECT_TRUE(r1.overlaps(r2));
}

TEST(RectangleTest, TestOverlapCorner)
{
  Point p0(0,0,0);
  Point p1(2,2,2);
  Rectangle r1(p0, p1);

  Point p2(1,1,1);
  Point p3(3,3,3);
  Rectangle r2(p2, p3);

  EXPECT_TRUE(r1.overlaps(r2));
}

TEST(RectangleTest, TestOverlapEdge)
{
  Point p0(0,0,0);
  Point p1(4,4,0);
  Rectangle r1(p0, p1);

  Point p2(1,-1,0);
  Point p3(3,1,0);
  Rectangle r2(p2, p3);

  EXPECT_TRUE(r1.overlaps(r2));
}

TEST(RectangleTest, TestOverlapIncludes)
{
  Point p0(0,0,0);
  Point p1(4,4,4);
  Rectangle r1(p0, p1);

  Point p2(2,2,2);
  Point p3(3,3,3);
  Rectangle r2(p2, p3);

  EXPECT_TRUE(r1.overlaps(r2));
}

TEST(RectangleTest, TestExpandToInclude)
{
  Rectangle r;
  r.expand_to_include(Point(1,2,3));
  EXPECT_EQ(0, r.p0.x);
  EXPECT_EQ(0, r.p0.y);
  EXPECT_EQ(0, r.p0.z);
  EXPECT_EQ(1, r.p1.x);
  EXPECT_EQ(2, r.p1.y);
  EXPECT_EQ(3, r.p1.z);

  // Already includes
  r.expand_to_include(Point(1,1,1));
  EXPECT_EQ(0, r.p0.x);
  EXPECT_EQ(0, r.p0.y);
  EXPECT_EQ(0, r.p0.z);
  EXPECT_EQ(1, r.p1.x);
  EXPECT_EQ(2, r.p1.y);
  EXPECT_EQ(3, r.p1.z);

  // Expand backwards
  r.expand_to_include(Point(-1,-2,-3));
  EXPECT_EQ(-1, r.p0.x);
  EXPECT_EQ(-2, r.p0.y);
  EXPECT_EQ(-3, r.p0.z);
  EXPECT_EQ(1, r.p1.x);
  EXPECT_EQ(2, r.p1.y);
  EXPECT_EQ(3, r.p1.z);
}

TEST(RectangleTest, TestBoundingBox)
{
  std::vector<Point> points
  {
    { 1, 1, 1 },
    { 2, 1, 1 },
    { 1, 3, 4 }
  };
  Rectangle r;
  r.become_bounding_box(points);
  EXPECT_EQ(1, r.p0.x);
  EXPECT_EQ(1, r.p0.y);
  EXPECT_EQ(1, r.p0.z);
  EXPECT_EQ(2, r.p1.x);
  EXPECT_EQ(3, r.p1.y);
  EXPECT_EQ(4, r.p1.z);
}

} // anon

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
