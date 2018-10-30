//==========================================================================
// ViGraph vector graphics: test-lines.cc
//
// Tests for lines
//
// Copyright (c) 2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-geometry.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
using namespace ViGraph::Geometry;

TEST(LineTest, TestLineInterpolation)
{
  Point p1(1,2,3);
  Point p2(5,6,7);
  Line l(p1, p2);
  Point p3 = l.interpolate(0.25);
  EXPECT_EQ(2, p3.x);
  EXPECT_EQ(3, p3.y);
  EXPECT_EQ(4, p3.z);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
