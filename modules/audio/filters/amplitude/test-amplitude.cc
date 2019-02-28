//==========================================================================
// ViGraph dataflow module: audio/sources/amplitude/test-amplitude.cc
//
// Tests for <amplitude> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module-test.h"
#include "vg-geometry.h"
#include <cmath>

ModuleLoader loader;
using namespace ViGraph::Geometry;

TEST(AmplitudeTest, TestNoWaveform)
{
  const string& xml = R"(
    <graph>
      <vco/>
      <amplitude property="gain"/>
      <attenuator gain="1.0"/>
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

TEST(AmplitudeTest, TestSquareWave)
{
  const string& xml = R"(
    <graph>
      <vco wave="square" freq="1"/>
      <amplitude target="a" property="gain"/>
      <attenuator id="a" gain="0.0"/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);
  Fragment *fragment = gen.get_fragment();
  ASSERT_FALSE(!fragment);

  const auto& waveform = fragment->waveforms[Speaker::front_center];

  // Should be 44100 samples at -1 or 1
  EXPECT_EQ(44100, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_EQ((i < 22050)?-1:1, waveform[i]) << i;
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  loader.load("../../sources/vco/vg-module-audio-source-vco.so");
  loader.load("../attenuator/vg-module-audio-filter-attenuator.so");
  loader.load("./vg-module-audio-filter-amplitude.so");
  return RUN_ALL_TESTS();
}
