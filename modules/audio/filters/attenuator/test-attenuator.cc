//==========================================================================
// ViGraph dataflow module: audio/sources/attenuator/test-attenuator.cc
//
// Tests for <attenuator> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module-test.h"
#include "vg-geometry.h"
#include <cmath>

ModuleLoader loader;
using namespace ViGraph::Geometry;

TEST(AttenuatorTest, TestNoWaveform)
{
  const string& xml = R"(
    <graph>
      <vco/>
      <attenuator gain="0.2" interpolate="false"/>
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

TEST(AttenuatorTest, TestSquareWaveReduced)
{
  const string& xml = R"(
    <graph>
      <vco wave="square" freq="1"/>
      <attenuator gain="0.2" interpolate="false"/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
  ASSERT_FALSE(!fragment);

  const auto& waveform = fragment->waveforms[Speaker::front_center];

  // Should be 44100 samples at alternating -1, 1
  EXPECT_EQ(44100, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_NEAR((i < 22050)?-0.2:0.2, waveform[i], 0.001)
      << i;
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  loader.load("../../sources/vco/vg-module-audio-source-vco.so");
  loader.load("./vg-module-audio-filter-attenuator.so");
  return RUN_ALL_TESTS();
}
