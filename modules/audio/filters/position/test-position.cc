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

  // Should be 88200 samples at 0
  EXPECT_EQ(88200, fragment->waveform.size());
  for(auto i=0u; i<fragment->waveform.size(); i++)
    EXPECT_EQ(0.0, fragment->waveform[i]);
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

  const auto centered = sin(pi / 4);

  // Should be 44100 samples at alternating -1, 1
  EXPECT_EQ(88200, fragment->waveform.size());
  for(auto i=0u; i<fragment->waveform.size(); i++)
    EXPECT_NEAR((i < 44100)?-centered:centered, fragment->waveform[i], 0.001)
      << i;
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

  // Should be 44100 samples at alternating -1, 1
  EXPECT_EQ(88200, fragment->waveform.size());
  for(auto i=0u; i<fragment->waveform.size(); i++)
    EXPECT_NEAR(((i < 44100)?-1:1) * (i % 2 ? right : left),
                fragment->waveform[i], 0.001) << i;
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

  // Should be 44100 samples at alternating -1, 1
  EXPECT_EQ(88200, fragment->waveform.size());
  for(auto i=0u; i<fragment->waveform.size(); i++)
    EXPECT_NEAR(((i < 44100)?-1:1) * (i % 2 ? right : left),
                fragment->waveform[i], 0.001) << i;
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

  // Should be 44100 samples at alternating -1, 1
  EXPECT_EQ(88200, fragment->waveform.size());
  for(auto i=0u; i<fragment->waveform.size(); i++)
    EXPECT_NEAR(((i < 44100)?-1:1) * (i % 2 ? right : left),
                fragment->waveform[i], 0.001) << i;
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

  // Should be 44100 samples at alternating -1, 1
  EXPECT_EQ(88200, fragment->waveform.size());
  for(auto i=0u; i<fragment->waveform.size(); i++)
    EXPECT_NEAR(((i < 44100)?-1:1) * (i % 2 ? right : left),
                fragment->waveform[i], 0.001) << i;
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  loader.load("../../sources/vco/vg-module-audio-source-vco.so");
  loader.load("./vg-module-audio-filter-position.so");
  return RUN_ALL_TESTS();
}
