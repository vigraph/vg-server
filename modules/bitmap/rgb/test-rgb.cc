//==========================================================================
// ViGraph dataflow module: bitmap/rgb/test-rgb.cc
//
// Tests for <rgb> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"
#include "../../module-test.h"

class RGBTest: public GraphTester
{
public:
  RGBTest()
  {
    loader.load("./vg-module-bitmap-rgb.so");
  }
};

const auto sample_rate = 1;

TEST_F(RGBTest, TestDefaultRGBIsBlack)
{
  auto& trans = add("bitmap/rgb");

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
    EXPECT_EQ(Colour::black, p);
}

TEST_F(RGBTest, TestSetRGivesRed)
{
  auto& trans = add("bitmap/rgb").set("r", 1.0);

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

TEST_F(RGBTest, TestSetGGivesGreen)
{
  auto& trans = add("bitmap/rgb").set("g", 1.0);

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
    EXPECT_EQ(Colour::green, p);
}

TEST_F(RGBTest, TestSetBGivesBlue)
{
  auto& trans = add("bitmap/rgb").set("b", 1.0);

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
    EXPECT_EQ(Colour::blue, p);
}

TEST_F(RGBTest, TestSetRGGivesYellow)
{
  auto& trans = add("bitmap/rgb").set("r", 1.0).set("g", 1.0);

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
    EXPECT_EQ(Colour::yellow, p);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
