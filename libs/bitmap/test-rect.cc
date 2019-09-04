//==========================================================================
// ViGraph bitmap graphics: test-bitmap.cc
//
// Tests for bitmap library
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-bitmap.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
using namespace std;


TEST(BitmapTest, TestDefaultConstructor)
{
  Bitmap::Rectangle b;
  EXPECT_EQ(1, b.get_width());
  EXPECT_EQ(0, b.get_height());
}

TEST(BitmapTest, TestHWConstructor)
{
  Bitmap::Rectangle b(3, 5);
  EXPECT_EQ(3, b.get_width());
  EXPECT_EQ(5, b.get_height());
}

TEST(BitmapTest, TestUnsetPixelsAreTransparent)
{
  Bitmap::Rectangle b(3, 5);
  EXPECT_TRUE(b(0,0).is_transparent());
}

TEST(BitmapTest, TestSetPixel)
{
  Bitmap::Rectangle b(3, 5);
  Colour::RGBA c(Colour::red, 0.5);
  b.set(2,3,c);
  EXPECT_EQ(c, b.get(2,3));
}

TEST(BitmapTest, TestFill)
{
  Bitmap::Rectangle b(3, 2);
  Colour::RGBA c(Colour::red, 0.5);
  b.fill(c);
  for(int i=0; i<b.get_height(); i++)
    for(int j=0; j<b.get_width(); j++)
      EXPECT_EQ(c, b.get(j,i));
}

TEST(BitmapTest, TestBlankToPPM)
{
  const string expected = R"(P3 3 2 255
0 0 0  0 0 0  0 0 0
0 0 0  0 0 0  0 0 0
)";
  Bitmap::Rectangle b(3, 2);
  string actual = b.to_ppm();
  EXPECT_EQ(expected, actual);
}

TEST(BitmapTest, TestSetToPPM)
{
  const string expected = R"(P3 3 2 255
255 255 255  0 0 0  255 0 0
0 0 0  0 255 0  0 0 255
)";
  Bitmap::Rectangle b(3, 2);
  b.set(0,0, Colour::white);
  b.set(2,0, Colour::red);
  b.set(1,1, Colour::green);
  b.set(2,1, Colour::blue);
  string actual = b.to_ppm();
  EXPECT_EQ(expected, actual);
}

TEST(BitmapTest, TestReadFromPPM)
{
  const string ppm = R"(P3 3 2 255
255 255 255  0 0 0  255 0 0
0 0 0  0 255 0  0 0 255
)";
  Bitmap::Rectangle b;
  ASSERT_NO_THROW(b.read_from_ppm(ppm));
  string actual = b.to_ppm();
  EXPECT_EQ(ppm, actual);
}

TEST(BitmapTest, TestASCIITestOutput)
{
  Bitmap::Rectangle b(3, 2);
  b.set(0,0, Colour::white);
  b.set(2,0, Colour::red);
  b.set(1,1, Colour::green);
  b.set(2,1, Colour::blue);

  const string ascii = R"(3x2
*_r
_gb
)";
  string actual = b.to_ascii();
  EXPECT_EQ(ascii, actual);
}

TEST(BitmapTest, TestFillSimplePolygon)
{
  Bitmap::Rectangle b(4, 4);
  Colour::RGBA c(Colour::red);

  vector<Geometry::Point> points;
  points.push_back({ -0.25, -0.25 });
  points.push_back({  0.25, -0.25, c });
  points.push_back({  0.25,  0.25, c });
  points.push_back({ -0.25,  0.25, c });

  b.fill_polygons(points);

  const string expected = R"(4x4
____
_rr_
_rr_
____
)";
  string actual = b.to_ascii();
  EXPECT_EQ(expected, actual);
}

TEST(BitmapTest, TestFillOffScreenPolygon)
{
  Bitmap::Rectangle b(4, 4);
  Colour::RGBA c(Colour::red);

  vector<Geometry::Point> points;
  points.push_back({ -0.25, -0.25 });
  points.push_back({  1.25, -0.25, c });
  points.push_back({  1.25,  1.25, c });
  points.push_back({ -0.25,  1.25, c });

  b.fill_polygons(points);

  const string expected = R"(4x4
_rrr
_rrr
_rrr
____
)";
  string actual = b.to_ascii();
  EXPECT_EQ(expected, actual);
}

TEST(BitmapTest, TestFillTrianglePolygon)
{
  Bitmap::Rectangle b(5, 5);
  Colour::RGBA c(Colour::white);

  vector<Geometry::Point> points;
  points.push_back({ -0.33, -0.33 });
  points.push_back({  0.33, -0.33, c });
  points.push_back({ -0.33,  0.33, c });

  b.fill_polygons(points);

  const string expected = R"(5x5
_____
_*___
_**__
_***_
_____
)";
  string actual = b.to_ascii();
  EXPECT_EQ(expected, actual);
}

TEST(BitmapTest, TestFillCombinedPolygon)
{
  Bitmap::Rectangle b(5, 5);
  Colour::RGBA cr(Colour::red);
  Colour::RGBA cg(Colour::green);

  vector<Geometry::Point> points;
  points.push_back({ -0.5, -0.5 });
  points.push_back({    0, -0.5, cr });
  points.push_back({    0,    0, cr });
  points.push_back({ -0.5,    0, cr });

  points.push_back({ -0.33, -0.33 });
  points.push_back({  0.33, -0.33, cg });
  points.push_back({ -0.33,  0.33, cg });

  b.fill_polygons(points);

  const string expected = R"(5x5
_____
_g___
rgg__
rggg_
rrr__
)";
  string actual = b.to_ascii();
  EXPECT_EQ(expected, actual);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
