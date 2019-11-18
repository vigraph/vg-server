//==========================================================================
// ViGraph dataflow module: laser/show-blanking/test-show-blanking.cc
//
// Tests for <show-blanking> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector/vector-module.h"
#include "../../module-test.h"

class ShowBlankingTest: public GraphTester
{
public:
  ShowBlankingTest()
  {
    loader.load("./vg-module-laser-show-blanking.so");
  }
};

const auto sample_rate = 1;

TEST_F(ShowBlankingTest, TestShowBlankDefault)
{
  auto& sb = add("laser/show-blanking");

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(0,0));
  fr.points.push_back(Point(1,0, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", sb, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  sb.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outfrs.size());
  const auto& outfr = outfrs[0];
  ASSERT_EQ(2, outfr.points.size());
  EXPECT_EQ(Colour::red, outfr.points[0].c);
  EXPECT_EQ(Colour::white, outfr.points[1].c);
}

TEST_F(ShowBlankingTest, TestShowBlankSpecified)
{
  auto& sb = add("laser/show-blanking").set("colour", string("#0f0"));

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(0,0));
  fr.points.push_back(Point(1,0, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", sb, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  sb.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outfrs.size());
  const auto& outfr = outfrs[0];
  ASSERT_EQ(2, outfr.points.size());
  EXPECT_EQ(Colour::green, outfr.points[0].c);
  EXPECT_EQ(Colour::white, outfr.points[1].c);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
