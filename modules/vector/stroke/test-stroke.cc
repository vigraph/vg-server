//==========================================================================
// ViGraph dataflow module: vector/stroke/test-stroke.cc
//
// Tests for <stroke> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include "../../colour/colour-module.h"
#include "../../module-test.h"

class StrokeTest: public GraphTester
{
public:
  StrokeTest()
  {
    loader.load("./vg-module-vector-stroke.so");
  }
};

const auto sample_rate = 1;

TEST_F(StrokeTest, TestDefaultStrokeIsWhite)
{
  auto& stroke = add("vector/stroke");

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(1,0, Colour::blue));

  auto& frs = add_source(fr_data);
  frs.connect("output", stroke, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  stroke.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outfrs.size());
  const auto& outfr = outfrs[0];
  ASSERT_EQ(1, outfr.points.size());
  EXPECT_EQ(Colour::white, outfr.points[0].c);
}

TEST_F(StrokeTest, TestStrokeWithInputIsSet)
{
  auto& stroke = add("vector/stroke");

  auto col_data = vector<Colour::RGB>(1);
  col_data[0] = Colour::red;
  auto& cols = add_source(col_data);
  cols.connect("output", stroke, "colour");

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(1,0, Colour::blue));

  auto& frs = add_source(fr_data);
  frs.connect("output", stroke, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  stroke.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outfrs.size());
  const auto& outfr = outfrs[0];
  ASSERT_EQ(1, outfr.points.size());
  EXPECT_EQ(Colour::red, outfr.points[0].c);
}

TEST_F(StrokeTest, TestBlankedPointsNotTouched)
{
  auto& stroke = add("vector/stroke");

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(1,0));

  auto& frs = add_source(fr_data);
  frs.connect("output", stroke, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  stroke.connect("output", snk, "input");

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
