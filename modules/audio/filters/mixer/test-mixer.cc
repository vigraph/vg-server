//==========================================================================
// ViGraph dataflow module: audio/sources/position/test-mixer.cc
//
// Tests for <mixer> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module-test.h"
#include "vg-geometry.h"
#include <cmath>

ModuleLoader loader;
using namespace ViGraph::Geometry;

TEST(MixerTest, TestNoWaveform)
{
  const string& xml = R"(
    <graph>
      <vco/>
      <mixer/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
  ASSERT_FALSE(!fragment);

  // Should be 44100 samples at 0
  EXPECT_EQ(44100, fragment->waveform.size());
  for(auto i=0u; i<fragment->waveform.size(); i++)
    EXPECT_EQ(0.0, fragment->waveform[i]);
}

TEST(MixerTest, TestSquareWave)
{
  const string& xml = R"(
    <graph>
      <vco wave="square" freq="1"/>
      <mixer/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
  ASSERT_FALSE(!fragment);

  // Should be 44100 samples at alternating -1, 1
  EXPECT_EQ(44100, fragment->waveform.size());
  for(auto i=0u; i<fragment->waveform.size(); i++)
    EXPECT_EQ((i < 22050)?-1:1, fragment->waveform[i]) << i;
}

TEST(MixerTest, TestTwoSquareWaves)
{
  const string& xml = R"(
    <graph>
      <vco wave="square" freq="1"/>
      <vco wave="square" freq="1"/>
      <mixer/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
  ASSERT_FALSE(!fragment);

  // Should be 44100 samples at alternating -2, 2
  EXPECT_EQ(44100, fragment->waveform.size());
  for(auto i=0u; i<fragment->waveform.size(); i++)
    EXPECT_EQ(((i < 22050)?-2:2), fragment->waveform[i]) << i;
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  loader.load("../../sources/vco/vg-module-audio-source-vco.so");
  loader.load("./vg-module-audio-filter-mixer.so");
  return RUN_ALL_TESTS();
}
