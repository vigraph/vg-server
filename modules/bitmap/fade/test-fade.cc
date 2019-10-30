//==========================================================================
// ViGraph dataflow module: bitmap/fade/test-fade.cc
//
// Tests for <fade> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"
#include "../../module-test.h"

class FadeTest: public GraphTester
{
public:
  FadeTest()
  {
    loader.load("./vg-module-bitmap-fade.so");
  }
};

const auto sample_rate = 1;

TEST_F(FadeTest, TestDefaultFadeHasNoEffect)
{
  auto& trans = add("bitmap/fade");

  auto bg_data = vector<Bitmap::Group>(1);
  auto& bg = bg_data[0];

  Bitmap::Rectangle r(5, 3);
  r.fill(Colour::red);
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

TEST_F(FadeTest, TestFade50)
{
  auto& trans = add("bitmap/fade").set("alpha", 0.5);

  auto bg_data = vector<Bitmap::Group>(1);
  auto& bg = bg_data[0];

  Bitmap::Rectangle r(5, 3);
  r.fill(Colour::red);
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
  Colour::RGBA half_red(Colour::red, 0.5);
  for(const auto& p: rect.get_pixels())
    EXPECT_EQ(half_red, p);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
