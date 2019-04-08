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
  const string& xml = R"(
    <graph>
      <vco/>
      <position/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
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
  const string& xml = R"(
    <graph>
      <vco wave="square" freq="1"/>
      <position/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
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
  const string& xml = R"(
    <graph>
      <vco wave="square" freq="1"/>
      <position x="0.5"/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
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
  const string& xml = R"(
    <graph>
      <vco wave="square" freq="1"/>
      <position x="-0.5"/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
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
  const string& xml = R"(
    <graph>
      <vco wave="square" freq="1"/>
      <position x="0.501"/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
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
  const string& xml = R"(
    <graph>
      <vco wave="square" freq="1"/>
      <position x="-0.501"/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
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
  return RUN_ALL_TESTS();
}
