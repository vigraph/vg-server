//==========================================================================
// ViGraph dataflow module: audio/sources/position/test-position.cc
//
// Tests for <position> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module-test.h"
#include "vg-geometry.h"
#include <cmath>

ModuleLoader loader;
using namespace ViGraph::Geometry;

TEST(PositionTest, TestNoWaveform)
{
  FragmentGraphTester tester{loader};

  auto vco = tester.add("vco");
  auto position = tester.add("position");

  vco.connect("default", position, "default");

  tester.run(2);

  const auto fragment = tester.get_fragment();
  ASSERT_FALSE(!fragment);

  ASSERT_EQ(2, fragment->waveforms.size());

  for (const auto& wit: fragment->waveforms)
  {
    const auto waveform = wit.second;
    // Should be 44100 samples at 0
    EXPECT_EQ(44100, waveform.size());
    for(auto i=0u; i<waveform.size(); i++)
      EXPECT_EQ(0.0, waveform[i]);
  }
}

TEST(PositionTest, TestSquareWaveCentered)
{
  FragmentGraphTester tester{loader};

  auto vco = tester.add("vco").set("wave", "square").set("freq", 1);
  auto position = tester.add("position");

  vco.connect("default", position, "default");

  tester.run(2);

  const auto fragment = tester.get_fragment();
  ASSERT_FALSE(!fragment);

  ASSERT_EQ(2, fragment->waveforms.size());

  const auto centered = sin(pi / 4);

  for (const auto& wit: fragment->waveforms)
  {
    const auto waveform = wit.second;
    // Should be 44100 samples at alternating -1, 1
    EXPECT_EQ(44100, waveform.size());
    for(auto i=0u; i<waveform.size(); i++)
      EXPECT_NEAR((i < 22050)?centered:-centered, waveform[i], 0.001) << i;
  }
}

TEST(PositionTest, TestSquareWaveFullRight)
{
  FragmentGraphTester tester{loader};

  auto vco = tester.add("vco").set("wave", "square").set("freq", 1);
  auto position = tester.add("position").set("x", 0.5);

  vco.connect("default", position, "default");

  tester.run(2);

  const auto fragment = tester.get_fragment();
  ASSERT_FALSE(!fragment);

  const auto left = 0.0;
  const auto right = 1.0;

  ASSERT_EQ(2, fragment->waveforms.size());

  const auto& wl = fragment->waveforms[Speaker::front_left];
  // Should be 44100 samples at alternating -1, 1
  EXPECT_EQ(44100, wl.size());
  for(auto i=0u; i<wl.size(); i++)
    EXPECT_NEAR(((i < 22050)?1:-1) * left, wl[i], 0.001) << i;

  const auto& wr = fragment->waveforms[Speaker::front_right];
  // Should be 44100 samples at alternating -1, 1
  EXPECT_EQ(44100, wr.size());
  for(auto i=0u; i<wr.size(); i++)
    EXPECT_NEAR(((i < 22050)?1:-1) * right, wr[i], 0.001) << i;
}

TEST(PositionTest, TestSquareWaveFullLeft)
{
  FragmentGraphTester tester{loader};

  auto vco = tester.add("vco").set("wave", "square").set("freq", 1);
  auto position = tester.add("position").set("x", -0.5);

  vco.connect("default", position, "default");

  tester.run(2);

  const auto fragment = tester.get_fragment();
  ASSERT_FALSE(!fragment);

  const auto left = 1.0;
  const auto right = 0.0;

  ASSERT_EQ(2, fragment->waveforms.size());

  const auto& wl = fragment->waveforms[Speaker::front_left];
  // Should be 44100 samples at alternating -1, 1
  EXPECT_EQ(44100, wl.size());
  for(auto i=0u; i<wl.size(); i++)
    EXPECT_NEAR(((i < 22050)?1:-1) * left, wl[i], 0.001) << i;

  const auto& wr = fragment->waveforms[Speaker::front_right];
  // Should be 44100 samples at alternating -1, 1
  EXPECT_EQ(44100, wr.size());
  for(auto i=0u; i<wr.size(); i++)
    EXPECT_NEAR(((i < 22050)?1:-1) * right, wr[i], 0.001) << i;
}

TEST(PositionTest, TestSquareWaveOutOfBoundsToTheRight)
{
  FragmentGraphTester tester{loader};

  auto vco = tester.add("vco").set("wave", "square").set("freq", 1);
  auto position = tester.add("position").set("x", 0.501);

  vco.connect("default", position, "default");

  tester.run(2);

  const auto fragment = tester.get_fragment();
  ASSERT_FALSE(!fragment);

  const auto left = 0.0;
  const auto right = 1.0;

  ASSERT_EQ(2, fragment->waveforms.size());

  const auto& wl = fragment->waveforms[Speaker::front_left];
  // Should be 44100 samples at alternating -1, 1
  EXPECT_EQ(44100, wl.size());
  for(auto i=0u; i<wl.size(); i++)
    EXPECT_NEAR(((i < 22050)?1:-1) * left, wl[i], 0.001) << i;

  const auto& wr = fragment->waveforms[Speaker::front_right];
  // Should be 44100 samples at alternating -1, 1
  EXPECT_EQ(44100, wr.size());
  for(auto i=0u; i<wr.size(); i++)
    EXPECT_NEAR(((i < 22050)?1:-1) * right, wr[i], 0.001) << i;
}

TEST(PositionTest, TestSquareWaveOutOfBoundsToTheLeft)
{
  FragmentGraphTester tester{loader};

  auto vco = tester.add("vco").set("wave", "square").set("freq", 1);
  auto position = tester.add("position").set("x", -0.501);

  vco.connect("default", position, "default");

  tester.run(2);

  const auto fragment = tester.get_fragment();
  ASSERT_FALSE(!fragment);

  const auto left = 1.0;
  const auto right = 0.0;

  ASSERT_EQ(2, fragment->waveforms.size());

  const auto& wl = fragment->waveforms[Speaker::front_left];
  // Should be 44100 samples at alternating -1, 1
  EXPECT_EQ(44100, wl.size());
  for(auto i=0u; i<wl.size(); i++)
    EXPECT_NEAR(((i < 22050)?1:-1) * left, wl[i], 0.001) << i;

  const auto& wr = fragment->waveforms[Speaker::front_right];
  // Should be 44100 samples at alternating -1, 1
  EXPECT_EQ(44100, wr.size());
  for(auto i=0u; i<wr.size(); i++)
    EXPECT_NEAR(((i < 22050)?1:-1) * right, wr[i], 0.001) << i;
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  loader.load("../../sources/vco/vg-module-audio-source-vco.so");
  loader.load("./vg-module-audio-filter-position.so");
  loader.add_default_section("audio");
  return RUN_ALL_TESTS();
}
