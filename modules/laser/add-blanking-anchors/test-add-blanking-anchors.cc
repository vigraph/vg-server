//==========================================================================
// ViGraph dataflow module:
//   laser/add-blanking-anchors/test-add-blanking-anchors.cc
//
// Tests for <add-blanking-anchors> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector/vector-module.h"
#include "../../module-test.h"

class AddBlankingAnchorsTest: public GraphTester
{
public:
  AddBlankingAnchorsTest()
  {
    loader.load("./vg-module-laser-add-blanking-anchors.so");
  }
};

const auto sample_rate = 1;

TEST_F(AddBlankingAnchorsTest, TestBlankingPointInsertion)
{
  auto& sb = add("laser/add-blanking-anchors")
    .set("leading", 1)
    .set("trailing", 2);

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(0,0));
  fr.points.push_back(Point(1,0, Colour::white));
  fr.points.push_back(Point(2,0));
  fr.points.push_back(Point(3,0, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", sb, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  sb.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outfrs.size());
  const auto& outfr = outfrs[0];
  const auto& opoints = outfr.points;
  // 1 at start, 2 added at 1, 1 at 2, 2 at end
  ASSERT_EQ(10, opoints.size());

  int i=0;
  // added leading at start
  EXPECT_EQ(Point(0,0), opoints[i]);
  EXPECT_TRUE(opoints[i++].is_blanked());
  // original
  EXPECT_EQ(Point(0,0), opoints[i]);
  EXPECT_TRUE(opoints[i++].is_blanked());
  // added trailing
  EXPECT_EQ(Point(1,0), opoints[i]);
  EXPECT_EQ(Colour::white, opoints[i++].c);
  EXPECT_EQ(Point(1,0), opoints[i]);
  EXPECT_EQ(Colour::white, opoints[i++].c);
  // original
  EXPECT_EQ(Point(1,0), opoints[i]);
  EXPECT_EQ(Colour::white, opoints[i++].c);
  // added leading
  EXPECT_EQ(Point(2,0), opoints[i]);
  EXPECT_TRUE(opoints[i++].is_blanked());
  // original
  EXPECT_EQ(Point(2,0), opoints[i]);
  EXPECT_TRUE(opoints[i++].is_blanked());
  // original
  EXPECT_EQ(Point(3,0), opoints[i]);
  EXPECT_EQ(Colour::white, opoints[i++].c);
  // added trailing at end
  EXPECT_EQ(Point(3,0), opoints[i]);
  EXPECT_EQ(Colour::white, opoints[i++].c);
  EXPECT_EQ(Point(3,0), opoints[i]);
  EXPECT_EQ(Colour::white, opoints[i++].c);

  EXPECT_EQ(10, i);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
