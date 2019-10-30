//==========================================================================
// ViGraph dataflow module: bitmap/hsl/test-hsl.cc
//
// Tests for <hsl> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"
#include "../../module-test.h"

class HSLTest: public GraphTester
{
public:
  HSLTest()
  {
    loader.load("./vg-module-bitmap-hsl.so");
  }
};

const auto sample_rate = 1;

TEST_F(HSLTest, TestDefaultHSLIsRed)
{
  auto& trans = add("bitmap/hsl");

  auto bg_data = vector<Bitmap::Group>(1);
  auto& bg = bg_data[0];

  Bitmap::Rectangle r(5, 3);
  r.fill(Colour::white);
  bg.add(r);

  auto& bgs = add_source(bg_data);
  bgs.connect("output", trans, "input");

  auto outbgs = vector<Bitmap::Group>{};
  auto& snk = add_sink(outbgs, sample_rate);
  trans.connect("output", snk, "input");

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
