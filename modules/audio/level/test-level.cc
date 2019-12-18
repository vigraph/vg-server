//==========================================================================
// ViGraph dataflow module: audio/level/test-level.cc
//
// Tests for audio level filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class LevelTest: public GraphTester
{
public:
  LevelTest()
  {
    loader.load("./vg-module-audio-level.so");
  }
};

const auto nsamples = 100;

TEST_F(LevelTest, TestSetOnlyInput)
{
  const auto expected = vector<Number>(nsamples, 42.0);
  auto actual = vector<Number>{};

  auto& mlt = add("audio/level")
              .set("input", 42.0);
  auto& snk = add_sink(actual, nsamples);
  mlt.connect("output", snk, "input");


  run();

  EXPECT_EQ(expected, actual);
}

TEST_F(LevelTest, TestSetOnlyGain)
{
  const auto expected = vector<Number>(nsamples, 0.0);
  auto actual = vector<Number>{};

  auto& mlt = add("audio/level")
              .set("gain", 10.0);
  auto& snk = add_sink(actual, nsamples);
  mlt.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected, actual);
}

TEST_F(LevelTest, TestSetBothInputAndGain)
{
  const auto expected = vector<Number>(nsamples, 4.2);
  auto actual = vector<Number>{};

  auto& mlt = add("audio/level")
              .set("input", 42.0)
              .set("gain", 0.1);
  auto& snk = add_sink(actual, nsamples);
  mlt.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected, actual);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
