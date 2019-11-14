//==========================================================================
// ViGraph dataflow module: vector/fade/test-fade.cc
//
// Tests for <fade> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../vector-module.h"
#include "../../module-test.h"

class FadeTest: public GraphTester
{
public:
  FadeTest()
  {
    loader.load("./vg-module-vector-fade.so");
  }
};

const auto sample_rate = 1;

TEST_F(FadeTest, TestDefaultFadeHasNoEffect)
{
  auto& fade = add("vector/fade");

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(1,0, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", fade, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  fade.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outfrs.size());
  const auto& outfr = outfrs[0];
  ASSERT_EQ(1, outfr.points.size());
  EXPECT_EQ(Colour::white, outfr.points[0].c);
}

TEST_F(FadeTest, TestFadeTo0_5)
{
  auto& fade = add("vector/fade").set("alpha", 0.5);

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(1,0, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", fade, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  fade.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outfrs.size());
  const auto& outfr = outfrs[0];
  ASSERT_EQ(1, outfr.points.size());
  EXPECT_EQ(Colour::RGB(0.5, 0.5, 0.5), outfr.points[0].c);
}

TEST_F(FadeTest, TestFadeToZeroIsBlanked)
{
  auto& fade = add("vector/fade").set("alpha", 0.0);

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(1,0, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", fade, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  fade.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outfrs.size());
  const auto& outfr = outfrs[0];
  ASSERT_EQ(1, outfr.points.size());
  EXPECT_TRUE(outfr.points[0].is_blanked());
}

TEST_F(FadeTest, TestBlankedPointsNotTouched)
{
  auto& fade = add("vector/fade");

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(1,0));

  auto& frs = add_source(fr_data);
  frs.connect("output", fade, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  fade.connect("output", snk, "input");

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
