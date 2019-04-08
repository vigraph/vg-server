//==========================================================================
// ViGraph dataflow module: audio/sources/position/test-combine.cc
//
// Tests for <combine> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module-test.h"
#include "vg-geometry.h"
#include <cmath>

ModuleLoader loader;
using namespace ViGraph::Geometry;

TEST(CombineTest, TestNoWaveform)
{
  const string& xml = R"(
    <graph>
      <vco/>
      <combine/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
  ASSERT_FALSE(!fragment);

  const auto& waveform = fragment->waveforms[Speaker::front_center];

  // Should be 44100 samples at 0
  EXPECT_EQ(44100, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_EQ(0.0, waveform[i]);
}

TEST(CombineTest, TestSquareWave)
{
  const string& xml = R"(
    <graph>
      <vco wave="square" freq="1"/>
      <combine/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
  ASSERT_FALSE(!fragment);

  // Should be 44100 samples at alternating -1, 1
  const auto& waveform = fragment->waveforms[Speaker::front_center];

  EXPECT_EQ(44100, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_EQ((i < 22050)?1:-1, waveform[i]) << i;
}

TEST(CombineTest, TestTwoSquareWaves)
{
  const string& xml = R"(
    <graph>
      <vco wave="square" freq="1"/>
      <vco wave="square" freq="1"/>
      <combine/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
  ASSERT_FALSE(!fragment);

  const auto& waveform = fragment->waveforms[Speaker::front_center];

  // Should be 44100 samples at alternating -2, 2
  EXPECT_EQ(44100, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_EQ(((i < 22050)?2:-2), waveform[i]) << i;
}

TEST(CombineTest, TestMultiplyTwoSineWaves)
{
  const string& xml = R"(
    <graph>
      <vco wave="sin" freq="1"/>
      <vco wave="sin" freq="1"/>
      <combine mode="multiply"/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
  ASSERT_FALSE(!fragment);

  const auto& waveform = fragment->waveforms[Speaker::front_center];

  // Should be 44100 samples at alternating -2, 2
  EXPECT_EQ(44100, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_NEAR(pow(sin(2*pi*(double)i/waveform.size()), 2),
                waveform[i], 0.0001) << i;
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  loader.load("../../sources/vco/vg-module-audio-source-vco.so");
  loader.load("./vg-module-audio-filter-combine.so");
  return RUN_ALL_TESTS();
}
