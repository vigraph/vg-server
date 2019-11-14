//==========================================================================
// ViGraph dataflow module: vector/hsl/test-hsl.cc
//
// Tests for <hsl> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include "../../module-test.h"

class HSLTest: public GraphTester
{
public:
  HSLTest()
  {
    loader.load("./vg-module-vector-hsl.so");
  }
};

const auto sample_rate = 1;

TEST_F(HSLTest, TestDefaultHSLIsRed)
{
  auto& hsl = add("vector/hsl");

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(1,0, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", hsl, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  hsl.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outfrs.size());
  const auto& outfr = outfrs[0];
  ASSERT_EQ(1, outfr.points.size());
  EXPECT_EQ(Colour::red, outfr.points[0].c);
}

TEST_F(HSLTest, TestHSLFullLuminanceIsWhite)
{
  auto& hsl = add("vector/hsl").set("l", 1.0);

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(1,0, Colour::blue));

  auto& frs = add_source(fr_data);
  frs.connect("output", hsl, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  hsl.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outfrs.size());
  const auto& outfr = outfrs[0];
  ASSERT_EQ(1, outfr.points.size());
  EXPECT_EQ(Colour::white, outfr.points[0].c);
}

TEST_F(HSLTest, TestBlankedPointsNotTouched)
{
  auto& hsl = add("vector/hsl");

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(1,0));

  auto& frs = add_source(fr_data);
  frs.connect("output", hsl, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  hsl.connect("output", snk, "input");

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
