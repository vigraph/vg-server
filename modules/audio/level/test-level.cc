//==========================================================================
// ViGraph dataflow module: audio/level/test-level.cc
//
// Tests for audio level filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
#include "../audio-module.h"

class LevelTest: public GraphTester
{
public:
  LevelTest()
  {
    loader.load("./vg-module-audio-level.so");
  }
};

TEST_F(LevelTest, TestSetBothInputAndGain)
{
  auto& level = add("audio/level");

  const auto input = vector<AudioData>{ -1, -0.8, -0.6, -0.4, -0.2, 0,
                                        0.2, 0.4, 0.6, 0.8, 1 };
  const auto gain = vector<Number>{ 1, 1, 1, 0.5, 0.5, 0.5,
                                    0, 0, 0, 0, 0 };
  const auto expected = vector<sample_t>{ -1, -0.8, -0.6, -0.2, -0.1, 0,
                                          0, 0, 0, 0, 0 };
  auto& isrc = add_source(input);
  auto& gsrc = add_source(gain);
  auto actual = vector<AudioData>{};
  auto& sink = add_sink(actual, input.size());

  isrc.connect("output", level, "input");
  gsrc.connect("output", level, "gain");
  level.connect("output", sink, "input");

  run();

  EXPECT_EQ(expected.size(), actual.size());
  for(auto i = 0u; i < actual.size(); ++i)
  {
    EXPECT_EQ(1, actual[i].nchannels);
    EXPECT_DOUBLE_EQ(expected[i], actual[i].channels[0]) << i;
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
