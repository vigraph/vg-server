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
  Bitmap::Bitmap b;
  EXPECT_EQ(1, b.get_width());
  EXPECT_EQ(0, b.get_height());
}

TEST(BitmapTest, TestHWConstructor)
{
  Bitmap::Bitmap b(3, 5);
  EXPECT_EQ(3, b.get_width());
  EXPECT_EQ(5, b.get_height());
}

TEST(BitmapTest, TestUnsetPixelsAreTransparent)
{
  Bitmap::Bitmap b(3, 5);
  EXPECT_TRUE(b(0,0).is_transparent());
}

TEST(BitmapTest, TestSetPixel)
{
  Bitmap::Bitmap b(3, 5);
  Colour::RGBA c(Colour::red, 0.5);
  b.set(2,3,c);
  EXPECT_EQ(c, b.get(2,3));
}

} // anonymous namespace

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
