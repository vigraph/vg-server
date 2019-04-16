//==========================================================================
// ViGraph vector graphics: test-colours.cc
//
// Tests for colours
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-colour.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
using namespace std;

TEST(ColourTest, TestColourDefaultConstruction)
{
  Colour::RGB c;
  EXPECT_EQ(0.0, c.r);
  EXPECT_EQ(0.0, c.g);
  EXPECT_EQ(0.0, c.b);
}

TEST(ColourTest, TestColourExplicitConstruction)
{
  Colour::RGB c(0.1,0.2,0.3);
  EXPECT_EQ(0.1, c.r);
  EXPECT_EQ(0.2, c.g);
  EXPECT_EQ(0.3, c.b);
}

TEST(ColourTest, TestColourMonoConstruction)
{
  Colour::RGB c(0.99);
  EXPECT_EQ(0.99, c.r);
  EXPECT_EQ(0.99, c.g);
  EXPECT_EQ(0.99, c.b);
}

TEST(ColourTest, TestHLSToRGBConversion)
{
  // Example from programmingalgorithms HSL->RGB code
  Colour::HSL hsl(138/360.0, 0.5, 0.76);
  Colour::RGB rgb(hsl);
  EXPECT_NEAR(0.639, rgb.r, 0.01);
  EXPECT_NEAR(0.878, rgb.g, 0.01);
  EXPECT_NEAR(0.710, rgb.b, 0.01);
}

TEST(ColourTest, TestHLSToRGBConversionGrey)
{
  // Example from programmingalgorithms HSL->RGB code
  Colour::HSL hsl(42.0, 0, 0.5);
  Colour::RGB rgb(hsl);
  EXPECT_NEAR(0.5, rgb.r, 0.01);
  EXPECT_NEAR(0.5, rgb.g, 0.01);
  EXPECT_NEAR(0.5, rgb.b, 0.01);
}

TEST(ColourTest, TestRGBToHSLConversion)
{
  // Example from programmingalgorithms RGB->HSL code
  Colour::RGB rgb = Colour::RGB::from_rgb_hex(82, 0, 87);
  Colour::HSL hsl(rgb);
  EXPECT_NEAR(296/360.0, hsl.h, 0.01);
  EXPECT_NEAR(1.0,       hsl.s, 0.01);
  EXPECT_NEAR(0.17,      hsl.l, 0.01);
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

TEST(ColourTest, TestHSLColourEquality)
{
  Colour::HSL c1(0.1,0.2,0.3);
  Colour::HSL c2(0.1,0.2,0.3);
  Colour::HSL c3(0.0,0.2,0.3);
  Colour::HSL c4(0.1,0.0,0.3);
  Colour::HSL c5(0.1,0.2,0.0);
  EXPECT_EQ(c1, c2);
  EXPECT_NE(c1, c3);
  EXPECT_NE(c1, c4);
  EXPECT_NE(c1, c5);
}

TEST(ColourTest, TestBogusColourStringThrows)
{
  ASSERT_THROW(Colour::RGB c("bogus"), runtime_error);
}

TEST(ColourTest, TestColour6HexStringConstructionWithHash)
{
  Colour::RGB c("#4080c0");
  EXPECT_NEAR(0.25, c.r, 0.01);
  EXPECT_NEAR(0.5,  c.g, 0.01);
  EXPECT_NEAR(0.75, c.b, 0.01);
}

TEST(ColourTest, TestColour6HexStringConstructionWithoutHash)
{
  Colour::RGB c("4080c0");
  EXPECT_NEAR(0.25, c.r, 0.01);
  EXPECT_NEAR(0.5,  c.g, 0.01);
  EXPECT_NEAR(0.75, c.b, 0.01);
}

TEST(ColourTest, TestColour3HexStringConstructionWithHash)
{
  Colour::RGB c("#48c");
  EXPECT_NEAR(0.26, c.r, 0.01); // 0x44
  EXPECT_NEAR(0.53, c.g, 0.01); // 0x88
  EXPECT_NEAR(0.80, c.b, 0.01); // 0xcc
}

TEST(ColourTest, TestColour3HexStringConstructionWithoutHash)
{
  Colour::RGB c("fff");
  EXPECT_DOUBLE_EQ(1.0, c.r);  // Important that 'f' is expanded to 'ff'
  EXPECT_DOUBLE_EQ(1.0, c.g);
  EXPECT_DOUBLE_EQ(1.0, c.b);
}

TEST(ColourTest, TestColourNameConstruction)
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

TEST(ColourTest, TestColourRGBFloatStringConstruction)
{
  Colour::RGB c("rgb(0.2,0.3, 0.4)");
  EXPECT_DOUBLE_EQ(0.2, c.r);
  EXPECT_DOUBLE_EQ(0.3, c.g);
  EXPECT_DOUBLE_EQ(0.4, c.b);
}

TEST(ColourTest, TestColourRGBIntegerStringConstruction)
{
  Colour::RGB c("RGB(64, 128, 255)");
  EXPECT_NEAR(0.25, c.r, 0.01);
  EXPECT_NEAR(0.5,  c.g, 0.01);
  EXPECT_DOUBLE_EQ(1.0, c.b);
}
TEST(ColourTest, TestColourRGBToString)
{
  Colour::RGB c1("#4080c0");
  EXPECT_EQ("#4080C0", c1.str());
  Colour::RGB c2(0,0,0);
  EXPECT_EQ("#000000", c2.str());
  Colour::RGB c3(1,1,1);
  EXPECT_EQ("#FFFFFF", c3.str());
}


TEST(ColourTest, TestColourHSLFloatStringConstruction)
{
  // Example from programmingalgorithms HSL->RGB code
  Colour::RGB c("hsl(0.383, 0.5, 0.76)");  // 138
  EXPECT_NEAR(0.639, c.r, 0.01);
  EXPECT_NEAR(0.878, c.g, 0.01);
  EXPECT_NEAR(0.710, c.b, 0.01);
}

TEST(ColourTest, TestColourHSLIntegerStringConstruction)
{
  // Example from online colour convertor
  Colour::RGB c("hsl(180, 50, 50)");
  EXPECT_NEAR(0.25, c.r, 0.01);
  EXPECT_NEAR(0.75, c.g, 0.01);
  EXPECT_NEAR(0.75, c.b, 0.01);
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

TEST(ColourTest, TestColourBlend)
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


} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
