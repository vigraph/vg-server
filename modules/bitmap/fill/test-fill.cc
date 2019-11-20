//==========================================================================
// ViGraph dataflow module: bitmap/fill/test-fill.cc
//
// Tests for <fill> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"
#include "../../colour/colour-module.h"
#include "../../module-test.h"

class FillTest: public GraphTester
{
public:
  FillTest()
  {
    loader.load("./vg-module-bitmap-fill.so");
  }
};

const auto sample_rate = 1;

TEST_F(FillTest, TestDefaultFillIsWhite)
{
  auto& fill = add("bitmap/fill");

  auto bg_data = vector<Bitmap::Group>(1);
  auto& bg = bg_data[0];

  Bitmap::Rectangle r(5, 3);
  r.fill(Colour::red);
  bg.add(r);

  auto& bgs = add_source(bg_data);
  bgs.connect("output", fill, "input");

  auto outbgs = vector<Bitmap::Group>{};
  auto& snk = add_sink(outbgs, sample_rate);
  fill.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outbgs.size());
  const auto& outbg = outbgs[0];
  ASSERT_EQ(1, outbg.items.size());
  auto rect = outbg.items[0].rect;
  for(const auto& p: rect.get_pixels())
    EXPECT_EQ(Colour::white, p);
}

TEST_F(FillTest, TestSpecifiedFill)
{
  auto& fill = add("bitmap/fill");

  auto col_data = vector<Colour::RGB>(1);
  col_data[0] = Colour::red;
  auto& cols = add_source(col_data);
  cols.connect("output", fill, "colour");

  auto bg_data = vector<Bitmap::Group>(1);
  auto& bg = bg_data[0];

  Bitmap::Rectangle r(5, 3);
  r.fill(Colour::red);
  bg.add(r);

  auto& bgs = add_source(bg_data);
  bgs.connect("output", fill, "input");

  auto outbgs = vector<Bitmap::Group>{};
  auto& snk = add_sink(outbgs, sample_rate);
  fill.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outbgs.size());
  const auto& outbg = outbgs[0];
  ASSERT_EQ(1, outbg.items.size());
  auto rect = outbg.items[0].rect;
  for(const auto& p: rect.get_pixels())
    EXPECT_EQ(Colour::red, p);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
