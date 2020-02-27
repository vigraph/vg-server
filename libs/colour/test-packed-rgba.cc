//==========================================================================
// ViGraph vector graphics: test-packed-rgba.cc
//
// Tests for packed RGB-Alpha colours
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "vg-colour.h"
#include <gtest/gtest.h>

namespace {

using namespace ViGraph;
using namespace std;

TEST(PackedRGBATest, TestRGBADefaultConstruction)
{
  Colour::PackedRGBA pc;
  EXPECT_EQ(0, pc.packed);
}

TEST(PackedRGBATest, TestRGBAExplicitConstruction)
{
  Colour::RGBA c(0,0.5,0.75,1.0);
  Colour::PackedRGBA pc(c);
  EXPECT_EQ(0xFFBF7F00ul, pc.packed);
}

TEST(PackedRGBATest, TestUnpacking)
{
  Colour::PackedRGBA pc(0xFFC08000ul);
  Colour::RGBA c = pc.unpack();
  EXPECT_EQ(0, c.r);
  EXPECT_NEAR(0.5, c.g, 0.01);
  EXPECT_NEAR(0.75, c.b, 0.01);
  EXPECT_EQ(1.0, c.a);
}

TEST(PackedRGBATest, TestEquality)
{
  Colour::PackedRGBA c1(Colour::red);
  Colour::PackedRGBA c2(Colour::red);
  Colour::PackedRGBA c3(Colour::green);
  EXPECT_EQ(c1, c2);
  EXPECT_NE(c1, c3);
}

TEST(PackedRGBATest, TestByteAccessors)
{
  Colour::PackedRGBA pc(0x01020304ul);
  EXPECT_EQ(1, pc.a8());
  EXPECT_EQ(2, pc.b8());
  EXPECT_EQ(3, pc.g8());
  EXPECT_EQ(4, pc.r8());
}

TEST(PackedRGBATest, TestOpaqueTransparent)
{
  Colour::PackedRGBA pc1(0xFF000000ul);
  ASSERT_TRUE(pc1.is_opaque());
  ASSERT_FALSE(pc1.is_transparent());

  Colour::PackedRGBA pc2(0x00000000ul);
  ASSERT_FALSE(pc2.is_opaque());
  ASSERT_TRUE(pc2.is_transparent());

  Colour::PackedRGBA pc3(0x80000000ul);
  ASSERT_FALSE(pc3.is_opaque());
  ASSERT_FALSE(pc3.is_transparent());
}

TEST(PackedRGBATest, TestBlend)
{
  Colour::RGB a(0.2, 0.4, 0.6);
  Colour::RGB b(0.4, 0.6, 0.8);
  Colour::RGBA b0(b, 0);
  Colour::RGBA b1(b, 1);
  Colour::PackedRGBA pa(a);
  Colour::PackedRGBA pb(b);
  Colour::PackedRGBA pb0(b0);
  Colour::PackedRGBA pb1(b1);
  EXPECT_EQ(pa, pb0.blend_with(pa));
  EXPECT_EQ(pb, pb1.blend_with(pa));

  Colour::RGBA bm(0.4, 0.6, 0.8, 0.5);
  Colour::PackedRGBA pbm(bm);
  Colour::PackedRGBA pc = pbm.blend_with(pa);
  Colour::RGB c = pc.unpack();
  EXPECT_NEAR(0.3, c.r, 0.01);
  EXPECT_NEAR(0.5, c.g, 0.01);
  EXPECT_NEAR(0.7, c.b, 0.01);
}

TEST(PackedRGBATest, TestRGBABlendOver)
{
  Colour::RGB a(0.2, 0.4, 0.6);
  Colour::RGB b(0.4, 0.6, 0.8);
  Colour::RGBA b0(b, 0);
  Colour::RGBA b1(b, 1);
  Colour::PackedRGBA pa(a);
  Colour::PackedRGBA pb(b);
  Colour::PackedRGBA pb0(b0);
  Colour::PackedRGBA pb1(b1);

  Colour::PackedRGBA pda(a);
  pb0.blend_over(pda);
  EXPECT_EQ(pa, pda);
  pb1.blend_over(pda);
  EXPECT_EQ(pb, pda);

  Colour::RGBA bm(0.4, 0.6, 0.8, 0.5);
  Colour::PackedRGBA pbm(bm);
  pda = pa;
  pbm.blend_over(pda);
  Colour::RGB c = pda.unpack();
  EXPECT_NEAR(0.3, c.r, 0.01);
  EXPECT_NEAR(0.5, c.g, 0.01);
  EXPECT_NEAR(0.7, c.b, 0.01);
}

TEST(PackedRGBATest, TestFade)
{
  Colour::PackedRGBA c(0x80010203ul);
  c.fade(0.5);
  Colour::PackedRGBA faded_c(0x40010203ul);
  EXPECT_EQ(faded_c, c);
}

TEST(PackedRGBATest, TestColourise)
{
  Colour::PackedRGBA c(0x80010203ul);
  Colour::RGB red(1.0, 0, 0);
  c.colourise(red);
  Colour::PackedRGBA new_c(0x800000FFul);
  EXPECT_EQ(new_c, c);
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
