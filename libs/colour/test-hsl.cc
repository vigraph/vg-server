//==========================================================================
// ViGraph vector graphics: test-hsl.cc
//
// Tests for HSL colours
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-colour.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
using namespace std;


TEST(ColourTest, TestHSLFloatStringConstruction)
{
  // Example from programmingalgorithms HSL->RGB code
  Colour::RGB c("hsl(0.383, 0.5, 0.76)");  // 138
  EXPECT_NEAR(0.639, c.r, 0.01);
  EXPECT_NEAR(0.878, c.g, 0.01);
  EXPECT_NEAR(0.710, c.b, 0.01);
}

TEST(ColourTest, TestHSLIntegerStringConstruction)
{
  // Example from online colour convertor
  Colour::RGB c("hsl(180, 50, 50)");
  EXPECT_NEAR(0.25, c.r, 0.01);
  EXPECT_NEAR(0.75, c.g, 0.01);
  EXPECT_NEAR(0.75, c.b, 0.01);
}

TEST(ColourTest, TestHSLToRGBConversion)
{
  // Example from programmingalgorithms HSL->RGB code
  Colour::HSL hsl(138/360.0, 0.5, 0.76);
  Colour::RGB rgb(hsl);
  EXPECT_NEAR(0.639, rgb.r, 0.01);
  EXPECT_NEAR(0.878, rgb.g, 0.01);
  EXPECT_NEAR(0.710, rgb.b, 0.01);
}

TEST(ColourTest, TestHSLToRGBConversionGrey)
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

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
