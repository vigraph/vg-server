//==========================================================================
// ViGraph dataflow module: laser/reorder-segments/test-reorder-segments.cc
//
// Tests for <reorder-segments> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector/vector-module.h"
#include "../../module-test.h"

class ReorderSegmentsTest: public GraphTester
{
public:
  ReorderSegmentsTest()
  {
    loader.load("./vg-module-laser-reorder-segments.so");
  }
};

const auto sample_rate = 1;

TEST_F(ReorderSegmentsTest, TestSegmentsInSensibleOrderNotReordered)
{
  auto& sb = add("laser/reorder-segments");

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(0,0));
  fr.points.push_back(Point(1,0, Colour::white));
  fr.points.push_back(Point(1,1));
  fr.points.push_back(Point(2,2, Colour::white));
  fr.points.push_back(Point(3,3));
  fr.points.push_back(Point(4,4, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", sb, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  sb.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outfrs.size());
  const auto& outfr = outfrs[0];
  ASSERT_EQ(6, outfr.points.size());
  EXPECT_EQ(0, outfr.points[0].x);
  EXPECT_EQ(1, outfr.points[1].x);
  EXPECT_EQ(1, outfr.points[2].x);
  EXPECT_EQ(2, outfr.points[3].x);
  EXPECT_EQ(3, outfr.points[4].x);
  EXPECT_EQ(4, outfr.points[5].x);
}

TEST_F(ReorderSegmentsTest, TestSegmentsInSillyOrderReordered)
{
  auto& sb = add("laser/reorder-segments");

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(0,0));
  fr.points.push_back(Point(1,0, Colour::white));
  fr.points.push_back(Point(3,3));
  fr.points.push_back(Point(4,4, Colour::white));
  fr.points.push_back(Point(1,1));
  fr.points.push_back(Point(2,2, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", sb, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  sb.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outfrs.size());
  const auto& outfr = outfrs[0];
  ASSERT_EQ(6, outfr.points.size());
  EXPECT_EQ(0, outfr.points[0].x);
  EXPECT_EQ(1, outfr.points[1].x);
  EXPECT_EQ(1, outfr.points[2].x);
  EXPECT_EQ(2, outfr.points[3].x);
  EXPECT_EQ(3, outfr.points[4].x);
  EXPECT_EQ(4, outfr.points[5].x);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
