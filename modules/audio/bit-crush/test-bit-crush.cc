//==========================================================================
// ViGraph dataflow module: core/bit-crush/test-bit-crush.cc
//
// Tests for <bit-crush> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
#include "vg-geometry.h"
#include "vg-waveform.h"

using namespace ViGraph::Geometry;

class BitCrushTest: public GraphTester
{
public:
  BitCrushTest()
  {
    loader.load("./vg-module-audio-bit-crush.so");
  }
};

const auto samples = 1;

TEST_F(BitCrushTest, TestNoInput)
{
  auto& btc = add("audio/bit-crush")
              .set("input", 0.0);
  auto actual = vector<double>{};
  auto& snk = add_sink(actual, samples);
  btc.connect("output", snk, "input");

  run();

  EXPECT_EQ(samples, actual.size());
  for(auto i = 0u; i < samples; ++i)
  {
    EXPECT_DOUBLE_EQ(0.0, actual[i]);
  }
}

TEST_F(BitCrushTest, TestBits)
{
  auto& btc = add("audio/bit-crush")
              .set("bits", 2.0);
  const auto input = vector<double>{-1, -0.8, -0.6, -0.4, -0.2, 0,
                                    0.2, 0.4, 0.6, 0.8, 1};
  const auto expected = vector<double>{-1, -1, -1.0/3.0, -1.0/3.0, -1.0/3.0,
                                       1.0/3.0, 1.0/3.0, 1.0/3.0, 1.0/3.0,
                                       1, 1};
  auto& src = add_source(input);
  auto actual = vector<double>{};
  auto& snk = add_sink(actual, input.size());
  src.connect("output", btc, "input");
  btc.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected.size(), actual.size());
  for(auto i = 0u; i < actual.size(); ++i)
  {
    EXPECT_DOUBLE_EQ(expected[i], actual[i]);
  }
}

TEST_F(BitCrushTest, TestSampleRate)
{
  auto& btc = add("audio/bit-crush")
              .set("rate", 2.0);
  const auto input = vector<double>{-1, -0.8, -0.6, -0.4, -0.2, 0, 0.2, 0.4};
  const auto expected = vector<double>{-1, -1, -1, -1, -0.2, -0.2, -0.2, -0.2};
  auto& src = add_source(input);
  auto actual = vector<double>{};
  auto& snk = add_sink(actual, input.size());
  src.connect("output", btc, "input");
  btc.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected.size(), actual.size());
  for(auto i = 0u; i < actual.size(); ++i)
  {
    EXPECT_DOUBLE_EQ(expected[i], actual[i]);
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
