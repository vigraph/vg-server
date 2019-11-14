//==========================================================================
// ViGraph dataflow module: laser/infill-lines/test-infill-lines.cc
//
// Tests for <infill-lines> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector/vector-module.h"
#include "../../module-test.h"

class InfillLinesTest: public GraphTester
{
public:
  InfillLinesTest()
  {
    loader.load("./vg-module-laser-infill-lines.so");
  }
};

const auto sample_rate = 1;

TEST_F(InfillLinesTest, TestMaximumDistancePointInsertion)
{
  auto& sb = add("laser/infill-lines").set("lit", 0.1);

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
  ASSERT_EQ(11, outfr.points.size()); // 11 fence posts, 10 rails
  double x = 0;
  for(const auto& p: outfr.points)
  {
    EXPECT_NEAR(x, p.x, 1e-5);
    x += 0.1;
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
  }
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
