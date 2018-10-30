//==========================================================================
// ViGraph vector graphics: test-bezier.cc
//
// Tests for Bezier curves
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-geometry.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
using namespace ViGraph::Geometry;

TEST(BezierTest, TestLineQuadraticBezierInterpolation)
{
  Point p0(0,0,0);
  Point p1(1,1,1);  // note, makes this linear
  Point p2(2,2,2);
  QuadraticBezier b(p0, p1, p2);

  // End points and centre, simple
  EXPECT_EQ(p0, b.interpolate(0));
  EXPECT_EQ(p1, b.interpolate(0.5));
  EXPECT_EQ(p2, b.interpolate(1));

  // Quarters
  Point p = b.interpolate(0.25);
  EXPECT_EQ(0.5, p.x);
  EXPECT_EQ(0.5, p.y);
  EXPECT_EQ(0.5, p.z);

  p = b.interpolate(0.75);
  EXPECT_EQ(1.5, p.x);
  EXPECT_EQ(1.5, p.y);
  EXPECT_EQ(1.5, p.z);
}

TEST(BezierTest, TestTriangleQuadraticBezierInterpolation)
{
  Point p0(0,0);
  Point p1(1,1);
  Point p2(2,0);
  QuadraticBezier b(p0, p1, p2);

  // End points
  EXPECT_EQ(p0, b.interpolate(0));
  EXPECT_EQ(p2, b.interpolate(1));

  // Mid point
  Point p = b.interpolate(0.5);
  EXPECT_EQ(1, p.x);
  EXPECT_EQ(0.5, p.y);
  EXPECT_EQ(0, p.z);
}

TEST(BezierTest, TestLineCubicBezierInterpolation)
{
  Point p0(0,0,0);
  Point p1(1,1,1);  // note, makes this linear
  Point p2(2,2,2);
  Point p3(3,3,3);
  CubicBezier b(p0, p1, p2, p3);

  // End points and centre, simple
  EXPECT_EQ(p0, b.interpolate(0));
  EXPECT_EQ(p3, b.interpolate(1));

  // Mid-point
  Point p = b.interpolate(0.5);
  EXPECT_EQ(1.5, p.x);
  EXPECT_EQ(1.5, p.y);
  EXPECT_EQ(1.5, p.z);

  // Thirds should be on control points
  p = b.interpolate(1/3.0);
  EXPECT_EQ(p1, p);

  p = b.interpolate(2/3.0);
  EXPECT_EQ(p2, p);
}

TEST(BezierTest, TestRectangleCubicBezierInterpolation)
{
  Point p0(0,0);
  Point p1(0,2);
  Point p2(2,2);
  Point p3(2,0);
  CubicBezier b(p0, p1, p2, p3);

  // End points and centre, simple
  EXPECT_EQ(p0, b.interpolate(0));
  EXPECT_EQ(p3, b.interpolate(1));

  // Mid-point
  Point p = b.interpolate(0.5);
  EXPECT_EQ(1, p.x);
  EXPECT_EQ(1.5, p.y);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
