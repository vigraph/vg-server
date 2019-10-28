//==========================================================================
// ViGraph bitmap graphics: test-group.cc
//
// Tests for bitmap groups
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-bitmap.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
using namespace ViGraph::Geometry;
using namespace std;

TEST(GroupTest, TestDefaultConstructor)
{
  Bitmap::Group group;
  auto bb = group.bounding_box();
  EXPECT_EQ(0, bb.p0.x);
  EXPECT_EQ(0, bb.p0.y);
  EXPECT_EQ(0, bb.p0.z);
  EXPECT_EQ(0, bb.p1.x);
  EXPECT_EQ(0, bb.p1.y);
  EXPECT_EQ(0, bb.p1.z);
}

TEST(GroupTest, TestBoundingBoxOfSingleRectAtOrigin)
{
  Bitmap::Group group;
  Bitmap::Rectangle rect(5, 3);
  group.add(rect);
  auto bb = group.bounding_box();
  EXPECT_EQ(0, bb.p0.x);
  EXPECT_EQ(0, bb.p0.y);
  EXPECT_EQ(0, bb.p0.z);
  EXPECT_EQ(5, bb.p1.x);
  EXPECT_EQ(3, bb.p1.y);
  EXPECT_EQ(0, bb.p1.z);
}

TEST(GroupTest, TestBoundingBoxOfSingleRectNotAtOrigin)
{
  Bitmap::Group group;
  Bitmap::Rectangle rect(5, 3);
  Vector pos(10,20,30);
  group.add(pos, rect);
  auto bb = group.bounding_box();
  EXPECT_EQ(10, bb.p0.x);
  EXPECT_EQ(20, bb.p0.y);
  EXPECT_EQ(30, bb.p0.z);
  EXPECT_EQ(15, bb.p1.x);
  EXPECT_EQ(23, bb.p1.y);
  EXPECT_EQ(30, bb.p1.z);
}

TEST(GroupTest, TestCompositionOfEmptyGroup)
{
  Bitmap::Group group;
  Bitmap::Rectangle comp = group.compose();
  EXPECT_EQ(0, comp.get_width());
  EXPECT_EQ(0, comp.get_height());
}

TEST(GroupTest, TestCompositionWithSingleRectAtOrigin)
{
  Bitmap::Group group;
  Bitmap::Rectangle rect(5, 3);
  rect.fill(Colour::white);
  group.add(rect);
  Bitmap::Rectangle comp = group.compose();
  EXPECT_EQ(5, comp.get_width());
  EXPECT_EQ(3, comp.get_height());
  for(int i=0; i<comp.get_height(); i++)
    for(int j=0; j<comp.get_width(); j++)
      EXPECT_EQ(Colour::white, comp.get(j,i));
}

TEST(GroupTest, TestCompositionWithSingleRectOffset)
{
  Bitmap::Group group;
  Bitmap::Rectangle rect(3, 2);
  rect.fill(Colour::white);
  group.add(Vector(2, 1), rect);
  Bitmap::Rectangle comp = group.compose();
  EXPECT_EQ(5, comp.get_width());
  EXPECT_EQ(3, comp.get_height());
  for(int i=0; i<comp.get_height(); i++)
    for(int j=0; j<comp.get_width(); j++)
      EXPECT_EQ((j<2 || i<1)?Colour::black:Colour::white,
                comp.get(j,i)) << j << "," << i;
}

TEST(GroupTest, TestCompositionWithSingleRectAlphaToSetBackground)
{
  Bitmap::Group group;
  Bitmap::Rectangle rect(5, 3);
  rect.fill(Colour::RGBA(Colour::red, 0.5));
  group.add(rect);
  Bitmap::Rectangle comp = group.compose(Colour::blue);
  EXPECT_EQ(5, comp.get_width());
  EXPECT_EQ(3, comp.get_height());

  Colour::RGBA combined(0.5, 0, 0.5, 1.0);
  for(int i=0; i<comp.get_height(); i++)
    for(int j=0; j<comp.get_width(); j++)
      EXPECT_EQ(combined, comp.get(j,i));
}

TEST(GroupTest, TestCompositionWithMultipleRectsZOrdered)
{
  Bitmap::Group group;

  Bitmap::Rectangle rect1(3, 2);
  rect1.fill(Colour::white);
  group.add(Vector(2,1,1), rect1);  // Note in front

  Bitmap::Rectangle rect2(5, 3);
  rect2.fill(Colour::black);
  group.add(Vector(0,0,0), rect2);  // Note background, needs reorder

  Bitmap::Rectangle comp = group.compose(Colour::blue); // Shouldn't be seen
  EXPECT_EQ(5, comp.get_width());
  EXPECT_EQ(3, comp.get_height());

  Colour::RGBA combined(0.5, 0, 0.5, 1.0);
  for(int i=0; i<comp.get_height(); i++)
    for(int j=0; j<comp.get_width(); j++)
      EXPECT_EQ((j<2 || i<1)?Colour::black:Colour::white,
                comp.get(j,i)) << j << "," << i;
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
