//==========================================================================
// ViGraph dataflow module: laser/add-vertex-repeats/test-add-vertex-repeats.cc
//
// Tests for <add-vertex-repeats> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector/vector-module.h"
#include "../../module-test.h"

class AddVertexRepeatsTest: public GraphTester
{
public:
  AddVertexRepeatsTest()
  {
    loader.load("./vg-module-laser-add-vertex-repeats.so");
  }
};

const auto sample_rate = 1;

TEST_F(AddVertexRepeatsTest, TestMaximumAngleVertexPointInsertion)
{
  auto& sb = add("laser/add-vertex-repeats")
    .set("max-angle", 30.0)
    .set("repeats", 5);

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(0,0));
  fr.points.push_back(Point(1,0, Colour::white));
  fr.points.push_back(Point(1,1, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", sb, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  sb.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outfrs.size());
  const auto& outfr = outfrs[0];
  ASSERT_EQ(8, outfr.points.size()); // 3 points, 5 added at vertex

  for(auto i=1; i<7; i++)
  {
    const auto& p = outfr.points[i];
    EXPECT_NEAR(1.0, p.x, 1e-5);
    EXPECT_NEAR(0.0, p.y, 1e-5);
  }
}

TEST_F(AddVertexRepeatsTest, TestMaximumAngleIgnoresSmallAngles)
{
  auto& sb = add("laser/add-vertex-repeats")
    .set("max-angle", 30.0)
    .set("repeats", 5);

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(0,0));
  fr.points.push_back(Point(1,0, Colour::white));
  fr.points.push_back(Point(2,0.5, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", sb, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  sb.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outfrs.size());
  const auto& outfr = outfrs[0];
  ASSERT_EQ(3, outfr.points.size()); // None added
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
