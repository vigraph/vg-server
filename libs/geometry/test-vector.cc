//==========================================================================
// ViGraph vector graphics: test-vector.cc
//
// Tests for vectors
//
// Copyright (c) 2017-2018 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-geometry.h"
#include <gtest/gtest.h>
#include <sstream>

namespace {

using namespace std;
using namespace ViGraph;
using namespace ViGraph::Geometry;

TEST(VectorTest, TestDefaultVectorConstructorIsNull)
{
  Vector v;
  EXPECT_EQ(0, v.x);
  EXPECT_EQ(0, v.y);
  EXPECT_EQ(0, v.z);
}

TEST(VectorTest, Test2DVectorConstruction)
{
  Vector v(1,2);
  EXPECT_EQ(1, v.x);
  EXPECT_EQ(2, v.y);
  EXPECT_EQ(0, v.z);
}

TEST(VectorTest, Test3DVectorConstruction)
{
  Vector v(1,2,3);
  EXPECT_EQ(1, v.x);
  EXPECT_EQ(2, v.y);
  EXPECT_EQ(3, v.z);
}

TEST(VectorTest, TestVectorSum)
{
  Vector v1(1,2,3);
  Vector v2(4,5,6);
  v1 += v2;
  EXPECT_EQ(5, v1.x);
  EXPECT_EQ(7, v1.y);
  EXPECT_EQ(9, v1.z);

  Vector v3 = v1+v2;
  EXPECT_EQ(9,  v3.x);
  EXPECT_EQ(12, v3.y);
  EXPECT_EQ(15, v3.z);
}

TEST(VectorTest, TestVectorDifference)
{
  Vector v1(4,5,6);
  Vector v2(1,2,3);
  v1 -= v2;
  EXPECT_EQ(3, v1.x);
  EXPECT_EQ(3, v1.y);
  EXPECT_EQ(3, v1.z);

  Vector v3 = v1-v2;
  EXPECT_EQ(2, v3.x);
  EXPECT_EQ(1, v3.y);
  EXPECT_EQ(0, v3.z);
}

TEST(VectorTest, TestVectorScale)
{
  Vector v1(2,4,6);
  Vector v2 = v1*0.5;
  EXPECT_EQ(1, v2.x);
  EXPECT_EQ(2, v2.y);
  EXPECT_EQ(3, v2.z);
  Vector v3 = v2;
  v2 *= 2;
  EXPECT_EQ(v1, v2);
  Vector v4 = v2/2;
  EXPECT_EQ(v3, v4);
  v2 /= 2;
  EXPECT_EQ(v4, v2);
}

TEST(VectorTest, TestVectorReverse)
{
  Vector v1(1,2,3);
  Vector v2 = -v1;
  EXPECT_EQ(-1, v2.x);
  EXPECT_EQ(-2, v2.y);
  EXPECT_EQ(-3, v2.z);
}

TEST(VectorTest, TestVectorEquality)
{
  Vector v1(1,2,3);
  Vector v2(1,2,3);
  Vector v3(1,2);
  EXPECT_EQ(v1, v2);
  EXPECT_NE(v1, v3);
}

TEST(VectorTest, TestVectorLength)
{
  Vector v1(0,0,0);
  EXPECT_EQ(0, v1.length());
  Vector v2(1,2,2);
  EXPECT_EQ(3, v2.length());
}

TEST(VectorTest, TestVectorDotProduct)
{
  Vector v1(1,2,3);
  Vector v2(10,100,1000);
  EXPECT_EQ(3210, v1.dot(v2));
  EXPECT_EQ(3210, v2.dot(v1));
}

TEST(VectorTest, TestVectorCrossProduct)
{
  // Example from https://rosettacode.org/wiki/Vector_products#C.2B.2B
  Vector v1(3,4,5);
  Vector v2(4,3,5);
  Vector c = v1.cross(v2);

  EXPECT_EQ(5, c.x);
  EXPECT_EQ(5, c.y);
  EXPECT_EQ(-7, c.z);
}

TEST(VectorTest, TestVectorAngle)
{
  Vector vx(1,0,0);
  Vector vy(0,2,0);  // making sure we handle non-unit vector
  Vector vz(0,0,3);
  EXPECT_EQ(0, vx.angle_to(vx));
  EXPECT_NEAR(pi/2, vx.angle_to(vy), 1e-5);
  EXPECT_NEAR(pi/2, vy.angle_to(vz), 1e-5);
  EXPECT_NEAR(pi/2, vx.angle_to(vz), 1e-5);

  // Only the 2D version works to give sign information
  EXPECT_NEAR(-pi/2, vx.angle_to(-vy), 1e-5);
}

TEST(VectorTest, TestVectorOutput2D)
{
  Vector v(1,2);
  ostringstream oss;
  oss << v;
  EXPECT_EQ("(1,2)", oss.str());
}

TEST(VectorTest, TestVectorOutput3D)
{
  Vector v(1,2,3);
  ostringstream oss;
  oss << v;
  EXPECT_EQ("(1,2,3)", oss.str());
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
