//==========================================================================
// ViGraph dataflow module: core/position/test-position.cc
//
// Tests for <position> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
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
  auto& pos = add("audio/position")
              .set("input", 0.0);
  auto left = vector<double>{};
  auto right = vector<double>{};
  auto& skl = add_sink(left, samples);
  auto& skr = add_sink(right, samples);
  pos.connect("left", skl, "input");
  pos.connect("right", skr, "input");

  run();

  EXPECT_EQ(samples, left.size());
  EXPECT_EQ(samples, right.size());
  for(auto i = 0u; i < samples; ++i)
  {
    EXPECT_DOUBLE_EQ(0.0, left[i]);
    EXPECT_DOUBLE_EQ(0.0, right[i]);
  }
}

TEST_F(PositionTest, TestCentered)
{
  auto& pos = add("audio/position")
              .set("input", 1.0)
              .set("x", 0.0);
  auto left = vector<double>{};
  auto right = vector<double>{};
  auto& skl = add_sink(left, samples);
  auto& skr = add_sink(right, samples);
  pos.connect("left", skl, "input");
  pos.connect("right", skr, "input");

  run();

  const auto centered = sin(pi / 4);

  EXPECT_EQ(samples, left.size());
  EXPECT_EQ(samples, right.size());
  for(auto i = 0u; i < samples; ++i)
  {
    EXPECT_DOUBLE_EQ(centered, left[i]);
    EXPECT_DOUBLE_EQ(centered, right[i]);
  }
}

TEST_F(PositionTest, TestFullRight)
{
  auto& pos = add("audio/position")
              .set("input", 1.0)
              .set("x", 0.5);
  auto left = vector<double>{};
  auto right = vector<double>{};
  auto& skl = add_sink(left, samples);
  auto& skr = add_sink(right, samples);
  pos.connect("left", skl, "input");
  pos.connect("right", skr, "input");

  run();

  const auto l = 0.0;
  const auto r = 1.0;

  EXPECT_EQ(samples, left.size());
  EXPECT_EQ(samples, right.size());
  for(auto i = 0u; i < samples; ++i)
  {
    EXPECT_NEAR(l, left[i], 1e-10);
    EXPECT_NEAR(r, right[i], 1e-10);
  }
}

TEST_F(PositionTest, TestFullLeft)
{
  auto& pos = add("audio/position")
              .set("input", 1.0)
              .set("x", -0.5);
  auto left = vector<double>{};
  auto right = vector<double>{};
  auto& skl = add_sink(left, samples);
  auto& skr = add_sink(right, samples);
  pos.connect("left", skl, "input");
  pos.connect("right", skr, "input");

  run();

  const auto l = 1.0;
  const auto r = 0.0;

  EXPECT_EQ(samples, left.size());
  EXPECT_EQ(samples, right.size());
  for(auto i = 0u; i < samples; ++i)
  {
    EXPECT_NEAR(l, left[i], 1e-10);
    EXPECT_NEAR(r, right[i], 1e-10);
  }
}

TEST_F(PositionTest, TestOutOfBoundsRight)
{
  auto& pos = add("audio/position")
              .set("input", 1.0)
              .set("x", 1.5);
  auto left = vector<double>{};
  auto right = vector<double>{};
  auto& skl = add_sink(left, samples);
  auto& skr = add_sink(right, samples);
  pos.connect("left", skl, "input");
  pos.connect("right", skr, "input");

  run();

  const auto l = 0.0;
  const auto r = 1.0;

  EXPECT_EQ(samples, left.size());
  EXPECT_EQ(samples, right.size());
  for(auto i = 0u; i < samples; ++i)
  {
    EXPECT_NEAR(l, left[i], 1e-10);
    EXPECT_NEAR(r, right[i], 1e-10);
  }
}

TEST_F(PositionTest, TestOutOfBoundsLeft)
{
  auto& pos = add("audio/position")
              .set("input", 1.0)
              .set("x", -1.5);
  auto left = vector<double>{};
  auto right = vector<double>{};
  auto& skl = add_sink(left, samples);
  auto& skr = add_sink(right, samples);
  pos.connect("left", skl, "input");
  pos.connect("right", skr, "input");

  run();

  const auto l = 1.0;
  const auto r = 0.0;

  EXPECT_EQ(samples, left.size());
  EXPECT_EQ(samples, right.size());
  for(auto i = 0u; i < samples; ++i)
  {
    EXPECT_NEAR(l, left[i], 1e-10);
    EXPECT_NEAR(r, right[i], 1e-10);
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
