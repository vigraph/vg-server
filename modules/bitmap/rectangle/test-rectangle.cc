//==========================================================================
// ViGraph dataflow module: vector/rectangle/test-rectangle.cc
//
// Tests for rectangle bitmap source
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"
#include "../../module-test.h"

ModuleLoader loader;

const auto sample_rate = 1;

TEST(RectangleTest, TestDefaultRectangleIs1x1White)
{
  GraphTester<Bitmap::Group> tester{loader, sample_rate};

  auto& rectangle = tester.add("bitmap/rectangle");

  tester.capture_from(rectangle, "output");

  tester.run();

  const auto bitmaps = tester.get_output();

  ASSERT_EQ(sample_rate, bitmaps.size());
  const auto& bitmap = bitmaps[0];
  ASSERT_EQ(1, bitmap.items.size());

  const auto& b0 = bitmap.items[0];
  EXPECT_EQ(0, b0.pos.x);
  EXPECT_EQ(0, b0.pos.y);
  EXPECT_EQ(0, b0.pos.z);
  EXPECT_EQ(1, b0.rect.get_width());
  EXPECT_EQ(1, b0.rect.get_height());
  EXPECT_EQ(Colour::white, b0.rect(0,0));
}

TEST(RectangleTest, TestSpecifiedHeightAndWidth)
{
  GraphTester<Bitmap::Group> tester{loader, sample_rate};

  auto& rectangle = tester.add("bitmap/rectangle")
                          .set("width", 5.0)
                          .set("height", 3.0);

  tester.capture_from(rectangle, "output");

  tester.run();

  const auto bitmaps = tester.get_output();

  ASSERT_EQ(sample_rate, bitmaps.size());
  const auto& bitmap = bitmaps[0];
  ASSERT_EQ(1, bitmap.items.size());

  const auto& b0 = bitmap.items[0];
  EXPECT_EQ(0, b0.pos.x);
  EXPECT_EQ(0, b0.pos.y);
  EXPECT_EQ(0, b0.pos.z);
  ASSERT_EQ(5, b0.rect.get_width());
  ASSERT_EQ(3, b0.rect.get_height());

  for(auto i=0u; i<5; i++)
    for(auto j=0u; j<3; j++)
      EXPECT_EQ(Colour::white, b0.rect(i,j));
}

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  loader.load("./vg-module-bitmap-rectangle.so");
  return RUN_ALL_TESTS();
}
