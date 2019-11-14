//==========================================================================
// ViGraph dataflow module: vector/rgb/test-rgb.cc
//
// Tests for <rgb> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include "../../module-test.h"

class RGBTest: public GraphTester
{
public:
  RGBTest()
  {
    loader.load("./vg-module-vector-rgb.so");
  }
};

const auto sample_rate = 1;

TEST_F(RGBTest, TestDefaultRGBIsBlack)
{
  auto& rgb = add("vector/rgb");

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(1,0, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", rgb, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  rgb.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outfrs.size());
  const auto& outfr = outfrs[0];
  ASSERT_EQ(1, outfr.points.size());
  EXPECT_EQ(Colour::black, outfr.points[0].c);
}

TEST_F(RGBTest, TestSetR)
{
  auto& rgb = add("vector/rgb").set("r", 1.0);

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(1,0, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", rgb, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  rgb.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outfrs.size());
  const auto& outfr = outfrs[0];
  ASSERT_EQ(1, outfr.points.size());
  EXPECT_EQ(Colour::red, outfr.points[0].c);
}

TEST_F(RGBTest, TestSetG)
{
  auto& rgb = add("vector/rgb").set("g", 1.0);

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(1,0, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", rgb, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  rgb.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outfrs.size());
  const auto& outfr = outfrs[0];
  ASSERT_EQ(1, outfr.points.size());
  EXPECT_EQ(Colour::green, outfr.points[0].c);
}

TEST_F(RGBTest, TestSetB)
{
  auto& rgb = add("vector/rgb").set("b", 1.0);

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(1,0, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", rgb, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  rgb.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outfrs.size());
  const auto& outfr = outfrs[0];
  ASSERT_EQ(1, outfr.points.size());
  EXPECT_EQ(Colour::blue, outfr.points[0].c);
}

TEST_F(RGBTest, TestBlankedPointsNotTouched)
{
  auto& rgb = add("vector/rgb").set("r", 1.0);

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(1,0));

  auto& frs = add_source(fr_data);
  frs.connect("output", rgb, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  rgb.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outfrs.size());
  const auto& outfr = outfrs[0];
  ASSERT_EQ(1, outfr.points.size());
  EXPECT_TRUE(outfr.points[0].is_blanked());
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
