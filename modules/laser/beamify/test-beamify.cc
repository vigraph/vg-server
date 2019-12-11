//==========================================================================
// ViGraph dataflow module: laser/beamify/test-beamify.cc
//
// Tests for <beamify> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector/vector-module.h"
#include "../../module-test.h"

class BeamifyTest: public GraphTester
{
public:
  BeamifyTest()
  {
    loader.load("./vg-module-laser-beamify.so");
  }
};

const auto sample_rate = 1;

TEST_F(BeamifyTest, TestDefaultDoesNothing)
{
  auto& sb = add("laser/beamify");

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
  EXPECT_EQ(Colour::black, outfr.points[0].c);
  EXPECT_EQ(Colour::white, outfr.points[1].c);
}

TEST_F(BeamifyTest, TestBeamifySpecified)
{
  auto& sb = add("laser/beamify")
            .set("every", Integer{2})
            .set("extra", Integer{1});

  auto fr_data = vector<Frame>(1);
  auto& fr = fr_data[0];
  fr.points.push_back(Point(0,0));
  fr.points.push_back(Point(1,0, Colour::red));
  fr.points.push_back(Point(2,0, Colour::white));

  auto& frs = add_source(fr_data);
  frs.connect("output", sb, "input");

  auto outfrs = vector<Frame>{};
  auto& snk = add_sink(outfrs, sample_rate);
  sb.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outfrs.size());
  const auto& outfr = outfrs[0];
  ASSERT_EQ(5, outfr.points.size());
  EXPECT_EQ(Colour::black, outfr.points[0].c);
  EXPECT_EQ(outfr.points[0], outfr.points[1]);
  EXPECT_EQ(Colour::red, outfr.points[2].c);
  EXPECT_EQ(Colour::white, outfr.points[3].c);
  EXPECT_EQ(outfr.points[3], outfr.points[4]);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
