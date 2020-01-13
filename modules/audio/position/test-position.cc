//==========================================================================
// ViGraph dataflow module: core/position/test-position.cc
//
// Tests for <position> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
#include "../audio-module.h"
#include "vg-geometry.h"
#include "vg-waveform.h"

using namespace ViGraph::Geometry;

class PositionTest: public GraphTester
{
public:
  PositionTest()
  {
    loader.load("./vg-module-audio-position.so");
  }
};

const auto samples = 1;

TEST_F(PositionTest, TestNoInput)
{
  auto& pos = add("audio/position");
  auto actual = vector<AudioData>{};
  auto& sink = add_sink(actual, samples);

  pos.connect("output", sink, "input");

  run();

  EXPECT_EQ(samples, actual.size());
  for(auto i = 0u; i < samples; ++i)
  {
    EXPECT_EQ(2, actual[i].nchannels);
    EXPECT_NEAR(0.0, actual[i].channels[0], 1e-20);
    EXPECT_NEAR(0.0, actual[i].channels[1], 1e-20);
  }
}

TEST_F(PositionTest, TestCentered)
{
  auto& pos = add("audio/position")
              .set("x", 0.0);

  const auto input = vector<AudioData>{ 1 };
  auto& isrc = add_source(input);
  isrc.connect("output", pos, "input");

  auto actual = vector<AudioData>{};
  auto& sink = add_sink(actual, samples);
  pos.connect("output", sink, "input");

  run();

  const auto centered = (float)sin(pi / 4);

  EXPECT_EQ(samples, actual.size());
  for(auto i = 0u; i < samples; ++i)
  {
    EXPECT_EQ(2, actual[i].nchannels);
    EXPECT_DOUBLE_EQ(centered, actual[i].channels[0]);
    EXPECT_DOUBLE_EQ(centered, actual[i].channels[1]);
  }
}

TEST_F(PositionTest, TestFullRight)
{
  auto& pos = add("audio/position")
              .set("x", 0.5);

  const auto input = vector<AudioData>{ 1 };
  auto& isrc = add_source(input);
  isrc.connect("output", pos, "input");

  auto actual = vector<AudioData>{};
  auto& sink = add_sink(actual, samples);
  pos.connect("output", sink, "input");

  run();

  const auto l = 0.0;
  const auto r = 1.0;

  EXPECT_EQ(samples, actual.size());
  for(auto i = 0u; i < samples; ++i)
  {
    EXPECT_EQ(2, actual[i].nchannels);
    EXPECT_NEAR(l, actual[i].channels[0], 1e-10);
    EXPECT_NEAR(r, actual[i].channels[1], 1e-10);
  }
}

TEST_F(PositionTest, TestFullLeft)
{
  auto& pos = add("audio/position")
              .set("x", -0.5);

  const auto input = vector<AudioData>{ 1 };
  auto& isrc = add_source(input);
  isrc.connect("output", pos, "input");

  auto actual = vector<AudioData>{};
  auto& sink = add_sink(actual, samples);
  pos.connect("output", sink, "input");

  run();

  const auto l = 1.0;
  const auto r = 0.0;

  EXPECT_EQ(samples, actual.size());
  for(auto i = 0u; i < samples; ++i)
  {
    EXPECT_EQ(2, actual[i].nchannels);
    EXPECT_NEAR(l, actual[i].channels[0], 1e-10);
    EXPECT_NEAR(r, actual[i].channels[1], 1e-10);
  }
}

TEST_F(PositionTest, TestOutOfBoundsRight)
{
  auto& pos = add("audio/position")
              .set("x", 1.5);

  const auto input = vector<AudioData>{ 1 };
  auto& isrc = add_source(input);
  isrc.connect("output", pos, "input");

  auto actual = vector<AudioData>{};
  auto& sink = add_sink(actual, samples);
  pos.connect("output", sink, "input");

  run();

  const auto l = 0.0;
  const auto r = 1.0;

  EXPECT_EQ(samples, actual.size());
  for(auto i = 0u; i < samples; ++i)
  {
    EXPECT_EQ(2, actual[i].nchannels);
    EXPECT_NEAR(l, actual[i].channels[0], 1e-10);
    EXPECT_NEAR(r, actual[i].channels[1], 1e-10);
  }
}

TEST_F(PositionTest, TestOutOfBoundsLeft)
{
  auto& pos = add("audio/position")
              .set("x", -1.5);

  const auto input = vector<AudioData>{ 1 };
  auto& isrc = add_source(input);
  isrc.connect("output", pos, "input");

  auto actual = vector<AudioData>{};
  auto& sink = add_sink(actual, samples);
  pos.connect("output", sink, "input");

  run();

  const auto l = 1.0;
  const auto r = 0.0;

  EXPECT_EQ(samples, actual.size());
  for(auto i = 0u; i < samples; ++i)
  {
    EXPECT_EQ(2, actual[i].nchannels);
    EXPECT_NEAR(l, actual[i].channels[0], 1e-10);
    EXPECT_NEAR(r, actual[i].channels[1], 1e-10);
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
