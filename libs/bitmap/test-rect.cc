//==========================================================================
// ViGraph bitmap graphics: test-rect.cc
//
// Tests for bitmap rectangles
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-bitmap.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
using namespace ViGraph::Geometry;
using namespace std;


TEST(RectangleTest, TestDefaultConstructor)
{
  Bitmap::Rectangle b;
  EXPECT_EQ(0, b.get_width());
  EXPECT_EQ(0, b.get_height());
}

TEST(RectangleTest, TestHWConstructor)
{
  Bitmap::Rectangle b(3, 5);
  EXPECT_EQ(3, b.get_width());
  EXPECT_EQ(5, b.get_height());
}

TEST(RectangleTest, TestUnsetPixelsAreTransparent)
{
  Bitmap::Rectangle b(3, 5);
  EXPECT_TRUE(b(0,0).is_transparent());
}

TEST(RectangleTest, TestSetPixel)
{
  Bitmap::Rectangle b(3, 5);
  Colour::RGBA c(Colour::red, 0.5);
  b.set(2,3,c);
  EXPECT_EQ(c, b.get(2,3));
}

TEST(RectangleTest, TestFill)
{
  Bitmap::Rectangle b(3, 2);
  Colour::RGBA c(Colour::red, 0.5);
  b.fill(c);
  for(int i=0; i<b.get_height(); i++)
    for(int j=0; j<b.get_width(); j++)
      EXPECT_EQ(c, b.get(j,i));
}

TEST(RectangleTest, TestBlankToPPM)
{
  const string expected = R"(P3 3 2 255
0 0 0  0 0 0  0 0 0
0 0 0  0 0 0  0 0 0
)";
  Bitmap::Rectangle b(3, 2);
  string actual = b.to_ppm();
  EXPECT_EQ(expected, actual);
}

TEST(RectangleTest, TestSetToPPM)
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

TEST(RectangleTest, TestReadFromPPM)
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

TEST(RectangleTest, TestASCIITestOutput)
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

TEST(RectangleTest, TestFillSimplePolygon)
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

TEST(RectangleTest, TestFillOffScreenPolygon)
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

TEST(RectangleTest, TestFillTrianglePolygon)
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

TEST(RectangleTest, TestFillCombinedPolygon)
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

TEST(RectangleTest, TestBlitSimple)
{
  Bitmap::Rectangle src(3, 2);
  Colour::RGBA c(Colour::red, 1.0);
  src.fill(c);

  Bitmap::Rectangle dest(3, 2);
  dest.fill(Colour::black);
  src.blit(Vector(), dest);

  for(int i=0; i<dest.get_height(); i++)
    for(int j=0; j<dest.get_width(); j++)
      EXPECT_EQ(c, dest.get(j,i));
}

TEST(RectangleTest, TestBlitOffset)
{
  Bitmap::Rectangle src(3, 2);
  Colour::RGBA cs(Colour::red, 1.0);
  src.fill(cs);

  Bitmap::Rectangle dest(5, 3);
  Colour::RGBA cd(Colour::blue, 1.0);
  dest.fill(cd);

  src.blit(Vector(2, 1), dest);

  for(int i=0; i<dest.get_height(); i++)
    for(int j=0; j<dest.get_width(); j++)
      EXPECT_EQ((j<2 || i<1)?cd:cs, dest.get(j,i)) << j << "," << i;
}

TEST(RectangleTest, TestBlitOffsetClippedBottomRight)
{
  Bitmap::Rectangle src(3, 2);
  Colour::RGBA cs(Colour::red, 1.0);
  src.fill(cs);

  Bitmap::Rectangle dest(5, 3);
  Colour::RGBA cd(Colour::blue, 1.0);
  dest.fill(cd);

  src.blit(Vector(3, 2), dest);

  for(int i=0; i<dest.get_height(); i++)
    for(int j=0; j<dest.get_width(); j++)
      EXPECT_EQ((j<3 || i<2)?cd:cs, dest.get(j,i)) << j << "," << i;
}

TEST(RectangleTest, TestBlitOffsetClippedTopLeft)
{
  Bitmap::Rectangle src(3, 2);
  Colour::RGBA cs(Colour::red, 1.0);
  src.fill(cs);

  Bitmap::Rectangle dest(5, 3);
  Colour::RGBA cd(Colour::blue, 1.0);
  dest.fill(cd);

  src.blit(Vector(-1, -1), dest);

  for(int i=0; i<dest.get_height(); i++)
    for(int j=0; j<dest.get_width(); j++)
      EXPECT_EQ((j>=2 || i>=1)?cd:cs, dest.get(j,i)) << j << "," << i;
}

TEST(RectangleTest, TestBlitOffsetClippedAllRound)
{
  Bitmap::Rectangle src(10, 10);
  Colour::RGBA cs(Colour::red, 1.0);
  src.fill(cs);

  Bitmap::Rectangle dest(5, 3);
  Colour::RGBA cd(Colour::blue, 1.0);
  dest.fill(cd);

  src.blit(Vector(-3, -3), dest);

  for(int i=0; i<dest.get_height(); i++)
    for(int j=0; j<dest.get_width(); j++)
      EXPECT_EQ(cs, dest.get(j,i)) << j << "," << i;
}

TEST(RectangleTest, TestBlitAlpha)
{
  Bitmap::Rectangle src(3, 2);
  Colour::RGBA cs(Colour::red, 0.5);
  src.fill(cs);

  Bitmap::Rectangle dest(3, 2);
  Colour::RGBA cd(Colour::blue, 1.0);
  dest.fill(cd);

  src.blit(Vector(), dest);

  Colour::RGBA combined(0.5, 0, 0.5, 1.0);

  for(int i=0; i<dest.get_height(); i++)
    for(int j=0; j<dest.get_width(); j++)
      EXPECT_EQ(combined, dest.get(j,i));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
