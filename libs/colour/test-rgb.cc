//==========================================================================
// ViGraph vector graphics: test-rgb.cc
//
// Tests for RGB colours
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-colour.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
using namespace std;

TEST(ColourTest, TestRGBDefaultConstruction)
{
  Colour::RGB c;
  EXPECT_EQ(0.0, c.r);
  EXPECT_EQ(0.0, c.g);
  EXPECT_EQ(0.0, c.b);
}

TEST(ColourTest, TestRGBExplicitConstruction)
{
  Colour::RGB c(0.1,0.2,0.3);
  EXPECT_EQ(0.1, c.r);
  EXPECT_EQ(0.2, c.g);
  EXPECT_EQ(0.3, c.b);
}

TEST(ColourTest, TestRGBMonoConstruction)
{
  Colour::RGB c(0.99);
  EXPECT_EQ(0.99, c.r);
  EXPECT_EQ(0.99, c.g);
  EXPECT_EQ(0.99, c.b);
}


TEST(ColourTest, TestRGBColourEquality)
{
  Colour::RGB c1(0.1,0.2,0.3);
  Colour::RGB c2(0.1,0.2,0.3);
  Colour::RGB c3(0.0,0.2,0.3);
  Colour::RGB c4(0.1,0.0,0.3);
  Colour::RGB c5(0.1,0.2,0.0);
  EXPECT_EQ(c1, c2);
  EXPECT_NE(c1, c3);
  EXPECT_NE(c1, c4);
  EXPECT_NE(c1, c5);
}

TEST(ColourTest, TestBogusColourStringThrows)
{
  ASSERT_THROW(Colour::RGB c("bogus"), runtime_error);
}

TEST(ColourTest, TestRGB6HexStringConstructionWithHash)
{
  Colour::RGB c("#4080c0");
  EXPECT_NEAR(0.25, c.r, 0.01);
  EXPECT_NEAR(0.5,  c.g, 0.01);
  EXPECT_NEAR(0.75, c.b, 0.01);
}

TEST(ColourTest, TestRGB6HexStringConstructionWithoutHash)
{
  Colour::RGB c("4080c0");
  EXPECT_NEAR(0.25, c.r, 0.01);
  EXPECT_NEAR(0.5,  c.g, 0.01);
  EXPECT_NEAR(0.75, c.b, 0.01);
}

TEST(ColourTest, TestRGB3HexStringConstructionWithHash)
{
  Colour::RGB c("#48c");
  EXPECT_NEAR(0.26, c.r, 0.01); // 0x44
  EXPECT_NEAR(0.53, c.g, 0.01); // 0x88
  EXPECT_NEAR(0.80, c.b, 0.01); // 0xcc
}

TEST(ColourTest, TestRGB3HexStringConstructionWithoutHash)
{
  Colour::RGB c("fff");
  EXPECT_DOUBLE_EQ(1.0, c.r);  // Important that 'f' is expanded to 'ff'
  EXPECT_DOUBLE_EQ(1.0, c.g);
  EXPECT_DOUBLE_EQ(1.0, c.b);
}

TEST(ColourTest, TestRGBNameConstruction)
{
  EXPECT_EQ(Colour::black,    Colour::RGB("black"));
  EXPECT_EQ(Colour::white,    Colour::RGB("white"));
  EXPECT_EQ(Colour::red,      Colour::RGB("RED"));
  EXPECT_EQ(Colour::green,    Colour::RGB("Lime"));
  EXPECT_EQ(Colour::blue,     Colour::RGB("#blue"));
  EXPECT_EQ(Colour::yellow,   Colour::RGB("yellow"));
  EXPECT_EQ(Colour::cyan,     Colour::RGB("cyan"));
  EXPECT_EQ(Colour::magenta,  Colour::RGB("magenta"));

  Colour::RGB teal("Teal");
  EXPECT_DOUBLE_EQ(0.0, teal.r);
  EXPECT_NEAR(0.5, teal.g, 0.01);
  EXPECT_NEAR(0.5, teal.b, 0.01);
}

TEST(ColourTest, TestRGBFloatStringConstruction)
{
  Colour::RGB c("rgb(0.2,0.3, 0.4)");
  EXPECT_DOUBLE_EQ(0.2, c.r);
  EXPECT_DOUBLE_EQ(0.3, c.g);
  EXPECT_DOUBLE_EQ(0.4, c.b);
}

TEST(ColourTest, TestRGBIntegerStringConstruction)
{
  Colour::RGB c("RGB(64, 128, 255)");
  EXPECT_NEAR(0.25, c.r, 0.01);
  EXPECT_NEAR(0.5,  c.g, 0.01);
  EXPECT_DOUBLE_EQ(1.0, c.b);
}
TEST(ColourTest, TestRGBToString)
{
  Colour::RGB c1("#4080c0");
  EXPECT_EQ("#4080C0", c1.str());
  Colour::RGB c2(0,0,0);
  EXPECT_EQ("#000000", c2.str());
  Colour::RGB c3(1,1,1);
  EXPECT_EQ("#FFFFFF", c3.str());
}

TEST(ColourTest, TestSpecialColours)
{
  EXPECT_EQ(0.0, Colour::black.r);
  EXPECT_EQ(0.0, Colour::black.g);
  EXPECT_EQ(0.0, Colour::black.b);

  EXPECT_EQ(1.0, Colour::white.r);
  EXPECT_EQ(1.0, Colour::white.g);
  EXPECT_EQ(1.0, Colour::white.b);

  EXPECT_EQ(1.0, Colour::red.r);
  EXPECT_EQ(0.0, Colour::red.g);
  EXPECT_EQ(0.0, Colour::red.b);

  EXPECT_EQ(0.0, Colour::green.r);
  EXPECT_EQ(1.0, Colour::green.g);
  EXPECT_EQ(0.0, Colour::green.b);

  EXPECT_EQ(0.0, Colour::blue.r);
  EXPECT_EQ(0.0, Colour::blue.g);
  EXPECT_EQ(1.0, Colour::blue.b);
}

TEST(ColourTest, TestRGBBlend)
{
  Colour::RGB a(0.2, 0.4, 0.6);
  Colour::RGB b(0.4, 0.6, 0.8);
  EXPECT_EQ(a, a.blend_with(b,0));
  EXPECT_EQ(b, a.blend_with(b,1));
  Colour::RGB c = a.blend_with(b,0.5);
  EXPECT_DOUBLE_EQ(0.3, c.r);
  EXPECT_DOUBLE_EQ(0.5, c.g);
  EXPECT_DOUBLE_EQ(0.7, c.b);
}

TEST(ColourTest, TestRGBAddition)
{
  Colour::RGB a(0.2, 0.4, 0.6);
  Colour::RGB b(0.4, 0.6, 0.8);
  a += b;
  EXPECT_DOUBLE_EQ(0.6, a.r);
  EXPECT_DOUBLE_EQ(1.0, a.g);
  EXPECT_DOUBLE_EQ(1.0, a.b);  // note fence
}


} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
