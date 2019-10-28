//==========================================================================
// ViGraph vector graphics: test-rgba.cc
//
// Tests for RGB-Alpha colours
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-colour.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
using namespace std;

TEST(ColourTest, TestRGBADefaultConstruction)
{
  Colour::RGBA c;
  EXPECT_EQ(0.0, c.r);
  EXPECT_EQ(0.0, c.g);
  EXPECT_EQ(0.0, c.b);
  EXPECT_EQ(0.0, c.a);
}

TEST(ColourTest, TestRGBAExplicitConstruction)
{
  Colour::RGBA c(0.1,0.2,0.3,0.4);
  EXPECT_EQ(0.1, c.r);
  EXPECT_EQ(0.2, c.g);
  EXPECT_EQ(0.3, c.b);
  EXPECT_EQ(0.4, c.a);
}

TEST(ColourTest, TestRGBAMonoConstruction)
{
  Colour::RGBA c(0.99, 0.5);
  EXPECT_EQ(0.99, c.r);
  EXPECT_EQ(0.99, c.g);
  EXPECT_EQ(0.99, c.b);
  EXPECT_EQ(0.5, c.a);
}

TEST(ColourTest, TestRGBATransparencyTests)
{
  Colour::RGBA c1(0,0);
  EXPECT_TRUE(c1.is_transparent());
  EXPECT_FALSE(c1.is_opaque());
  Colour::RGBA c2(0,1);
  EXPECT_FALSE(c2.is_transparent());
  EXPECT_TRUE(c2.is_opaque());
  Colour::RGBA c3(0,0.5);
  EXPECT_FALSE(c3.is_transparent());
  EXPECT_FALSE(c3.is_opaque());
}

TEST(ColourTest, TestRGBAColourEquality)
{
  Colour::RGBA c1(0.1,0.2,0.3,0.4);
  Colour::RGBA c2(0.1,0.2,0.3,0.4);
  Colour::RGBA c3(0.1,0.2,0.3,0.5);
  Colour::RGBA c4(0.0,0.2,0.3,0.4);
  EXPECT_EQ(c1, c2);
  EXPECT_NE(c1, c3);
  EXPECT_NE(c1, c4);
}

TEST(ColourTest, TestBogusColourStringThrows)
{
  ASSERT_THROW(Colour::RGBA c("bogus"), runtime_error);
}

TEST(ColourTest, TestRGBA8HexStringConstructionWithHash)
{
  Colour::RGBA c("#4080c080");
  EXPECT_NEAR(0.25, c.r, 0.01);
  EXPECT_NEAR(0.5,  c.g, 0.01);
  EXPECT_NEAR(0.75, c.b, 0.01);
  EXPECT_NEAR(0.5,  c.a, 0.01);
}

TEST(ColourTest, TestRGBA8HexStringConstructionWithoutHash)
{
  Colour::RGBA c("4080c080");
  EXPECT_NEAR(0.25, c.r, 0.01);
  EXPECT_NEAR(0.5,  c.g, 0.01);
  EXPECT_NEAR(0.75, c.b, 0.01);
  EXPECT_NEAR(0.5,  c.a, 0.01);
}

TEST(ColourTest, TestRGBA4HexStringConstructionWithHash)
{
  Colour::RGBA c("#48c8");
  EXPECT_NEAR(0.26, c.r, 0.01); // 0x44
  EXPECT_NEAR(0.53, c.g, 0.01); // 0x88
  EXPECT_NEAR(0.80, c.b, 0.01); // 0xcc
  EXPECT_NEAR(0.53,  c.a, 0.01);// 0x88
}

TEST(ColourTest, TestRGBA4HexStringConstructionWithoutHash)
{
  Colour::RGBA c("ffff");
  EXPECT_DOUBLE_EQ(1.0, c.r);  // Important that 'f' is expanded to 'ff'
  EXPECT_DOUBLE_EQ(1.0, c.g);
  EXPECT_DOUBLE_EQ(1.0, c.b);
  EXPECT_DOUBLE_EQ(1.0, c.a);
}

TEST(ColourTest, TestRGBAFloatStringConstructionWithAlpha)
{
  Colour::RGBA c("rgb(0.2,0.3, 0.4)", 0.5);
  EXPECT_DOUBLE_EQ(0.2, c.r);
  EXPECT_DOUBLE_EQ(0.3, c.g);
  EXPECT_DOUBLE_EQ(0.4, c.b);
  EXPECT_DOUBLE_EQ(0.5, c.a);
}

TEST(ColourTest, TestRGBAToString)
{
  Colour::RGBA c1("#4080c080");
  EXPECT_EQ("#4080C080", c1.str());
  Colour::RGBA c2(0,0,0,0);
  EXPECT_EQ("#00000000", c2.str());
  Colour::RGBA c3(1,1,1,1);
  EXPECT_EQ("#FFFFFFFF", c3.str());
}

TEST(ColourTest, TestRGBABlend)
{
  Colour::RGB a(0.2, 0.4, 0.6);
  Colour::RGB b(0.4, 0.6, 0.8);
  Colour::RGBA b0(b, 0);
  Colour::RGBA b1(b, 1);
  EXPECT_EQ(a, b0.blend_with(a));
  EXPECT_EQ(b, b1.blend_with(a));

  Colour::RGBA bm(0.4, 0.6, 0.8, 0.5);
  Colour::RGB c = bm.blend_with(a);
  EXPECT_DOUBLE_EQ(0.3, c.r);
  EXPECT_DOUBLE_EQ(0.5, c.g);
  EXPECT_DOUBLE_EQ(0.7, c.b);
}

TEST(ColourTest, TestRGBABlendOver)
{
  Colour::RGB a(0.2, 0.4, 0.6);
  Colour::RGB b(0.4, 0.6, 0.8);
  Colour::RGBA b0(b, 0);
  Colour::RGBA b1(b, 1);

  Colour::RGB da = a;
  b0.blend_over(da);
  EXPECT_EQ(a, da);
  b1.blend_over(da);
  EXPECT_EQ(b, da);

  Colour::RGBA bm(0.4, 0.6, 0.8, 0.5);
  da = a;
  bm.blend_over(da);
  EXPECT_DOUBLE_EQ(0.3, da.r);
  EXPECT_DOUBLE_EQ(0.5, da.g);
  EXPECT_DOUBLE_EQ(0.7, da.b);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
