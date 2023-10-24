//==========================================================================
// ViGraph dataflow module: laser/strip-blank-runs/test-strip-blank-runs.cc
//
// Tests for <strip-blank-runs> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector/vector-module.h"
#include "../../module-test.h"

class StripBlankRunsTest: public GraphTester
{
public:
  StripBlankRunsTest()
  {
    loader.load("./vg-module-laser-strip-blank-runs.so");
  }
};

const auto sample_rate = 1;

TEST_F(StripBlankRunsTest, TestBlanksStrippedAboveThreshold)
{
  auto& sb = add("laser/strip-blank-runs").set("threshold", 5.0);

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];

  for(int i=0; i<10; i++)
    fr.points.push_back(Point(0, 0));
  for(int i=0; i<10; i++)
    fr.points.push_back(Point(0, 0, Colour::white));
  for(int i=0; i<5; i++)
    fr.points.push_back(Point(0, 0));
  fr.points.push_back(Point(0, 0, Colour::white));
  for(int i=0; i<10; i++)
    fr.points.push_back(Point(0, 0));

  auto& frs = add_source(fr_data);
  frs.connect("output", sb, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  sb.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outfrs.size());
  const auto& outfr = outfrs[0];
  ASSERT_EQ(26, outfr.points.size());

  for(int i=0; i<5; i++)
    EXPECT_TRUE(outfr.points[i].is_blanked());
  for(int i=5; i<15; i++)
    EXPECT_TRUE(outfr.points[i].is_lit());
  for(int i=15; i<20; i++)
    EXPECT_TRUE(outfr.points[i].is_blanked());
  EXPECT_TRUE(outfr.points[20].is_lit());
  for(int i=21; i<26; i++)
    EXPECT_TRUE(outfr.points[i].is_blanked());
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
