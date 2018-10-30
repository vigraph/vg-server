//==========================================================================
// ViGraph SVG library: test-path-render.cc
//
// Tests for path to points rendering
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-svg.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
using namespace ViGraph::SVG;

TEST(PathRenderTest, TestRenderEmptyPath)
{
  Path path;
  ASSERT_NO_THROW(path.read(""));

  vector<Point> points;
  path.render(points);
  ASSERT_TRUE(points.empty());
}

TEST(PathRenderTest, TestRenderSingleMove)
{
  Path path;
  ASSERT_NO_THROW(path.read("m1 2"));

  vector<Point> points;
  path.render(points);
  ASSERT_EQ(1, points.size());
  Point& p = points[0];
  ASSERT_EQ(1, p.x);
  ASSERT_EQ(2, p.y);
  ASSERT_EQ(0, p.z);
  ASSERT_TRUE(p.is_blanked());
}

TEST(PathRenderTest, TestRenderClosepath)
{
  Path path;
  ASSERT_NO_THROW(path.read("m0 0 1 2 z"));

  vector<Point> points;
  path.render(points);
  ASSERT_EQ(3, points.size());
  Point& p0 = points[0];
  ASSERT_EQ(0, p0.x);
  ASSERT_EQ(0, p0.y);
  ASSERT_EQ(0, p0.z);
  ASSERT_TRUE(p0.is_blanked());

  Point& p1 = points[1];
  ASSERT_EQ(1, p1.x);
  ASSERT_EQ(2, p1.y);
  ASSERT_EQ(0, p1.z);
  ASSERT_TRUE(p1.is_lit());

  Point& p2 = points[2];
  ASSERT_EQ(0, p2.x);
  ASSERT_EQ(0, p2.y);
  ASSERT_EQ(0, p2.z);
  ASSERT_TRUE(p2.is_lit());
}

TEST(PathRenderTest, TestRenderSimpleLine)
{
  Path path;
  ASSERT_NO_THROW(path.read("m 0,0 1,2"));

  vector<Point> points;
  path.render(points);
  ASSERT_EQ(2, points.size());
  Point& p1 = points[0];
  ASSERT_EQ(0, p1.x);
  ASSERT_EQ(0, p1.y);
  ASSERT_EQ(0, p1.z);
  ASSERT_TRUE(p1.is_blanked());

  Point& p2 = points[1];
  ASSERT_EQ(1, p2.x);
  ASSERT_EQ(2, p2.y);
  ASSERT_EQ(0, p2.z);
  ASSERT_TRUE(p2.is_lit());
}

TEST(PathRenderTest, TestRenderHLine)
{
  Path path;
  ASSERT_NO_THROW(path.read("m 0,0 h 1"));

  vector<Point> points;
  path.render(points);
  ASSERT_EQ(2, points.size());

  // Not going to repeat check for first blank point or 'z' from now on

  Point& p = points[1];
  ASSERT_EQ(1, p.x);
  ASSERT_EQ(0, p.y);
  ASSERT_TRUE(p.is_lit());
}

TEST(PathRenderTest, TestRenderVLine)
{
  Path path;
  ASSERT_NO_THROW(path.read("m 0,0 v 1"));

  vector<Point> points;
  path.render(points);
  ASSERT_EQ(2, points.size());

  Point& p = points[1];
  ASSERT_EQ(0, p.x);
  ASSERT_EQ(1, p.y);
  ASSERT_TRUE(p.is_lit());
}

TEST(PathRenderTest, TestRenderCurve)
{
  Path path;
  ASSERT_NO_THROW(path.read("m 0,0 c 1,1 2,2 3,3"));

  vector<Point> points;
  path.render(points);
  ASSERT_EQ(11, points.size());

  // !!! What else to check?
}

TEST(PathRenderTest, TestRenderSmoothCurve)
{
  Path path;
  ASSERT_NO_THROW(path.read("m 0,0 s 2,2 3,3"));

  vector<Point> points;
  path.render(points);
  ASSERT_EQ(11, points.size());
}

TEST(PathRenderTest, TestRenderQuadratic)
{
  Path path;
  ASSERT_NO_THROW(path.read("m 0,0 q 1,1 2,2"));

  vector<Point> points;
  path.render(points);
  ASSERT_EQ(11, points.size());
}

TEST(PathRenderTest, TestRenderSmoothQuadratic)
{
  Path path;
  ASSERT_NO_THROW(path.read("m 0,0 t 2,2"));

  vector<Point> points;
  path.render(points);
  ASSERT_EQ(11, points.size());
}

#if 0
  // Not yet implemented
TEST(PathRenderTest, TestRenderEllipticalArc)
{
  Path path;
  ASSERT_NO_THROW(path.read("m 0,0 a 1,1 -30 1 0 2,2"));

  vector<Point> points;
  path.render(points);
  ASSERT_EQ(11, points.size());
}
#endif

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
