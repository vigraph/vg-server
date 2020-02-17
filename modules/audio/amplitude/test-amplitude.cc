//==========================================================================
// ViGraph dataflow module: audio/amplitude/test-amplitude.cc
//
// Tests for audio amplitude filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
#include "../audio-module.h"

class AmplitudeTest: public GraphTester
{
public:
  AmplitudeTest()
  {
    loader.load("./vg-module-audio-amplitude.so");
  }
};

TEST_F(AmplitudeTest, TestExtractionValidChannel)
{
  auto& amp = add("audio/amplitude");

  const auto input = vector<AudioData>{ -1, 0, 1 };
  const auto expected = vector<Number>{ 0, 0.5, 1 };
  auto& isrc = add_source(input);
  isrc.connect("output", amp, "input");

  auto actual = vector<Number>{};
  auto& sink = add_sink(actual, input.size());
  amp.connect("output", sink, "input");

  run();

  EXPECT_EQ(expected.size(), actual.size());
  for(auto i = 0u; i < actual.size(); ++i)
    EXPECT_DOUBLE_EQ(expected[i], actual[i]) << i;
}

TEST_F(AmplitudeTest, TestExtractionInvalidChannel)
{
  auto& amp = add("audio/amplitude").set("channel", Integer{1});

  const auto input = vector<AudioData>{ -1, 0, 1 };
  const auto expected = vector<Number>{ 0, 0, 0 };
  auto& isrc = add_source(input);
  isrc.connect("output", amp, "input");

  auto actual = vector<Number>{};
  auto& sink = add_sink(actual, input.size());
  amp.connect("output", sink, "input");

  run();

  EXPECT_EQ(expected.size(), actual.size());
  for(auto i = 0u; i < actual.size(); ++i)
    EXPECT_DOUBLE_EQ(expected[i], actual[i]) << i;
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
