//==========================================================================
// ViGraph dataflow module: audio/sources/vu-meter/test-vu-meter.cc
//
// Tests for <vu-meter> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module-test.h"
#include "vg-geometry.h"
#include <cmath>

ModuleLoader loader;
using namespace ViGraph::Geometry;

TEST(VUMeterTest, TestNoWaveform)
{
  const string& xml = R"(
    <graph>
      <vco/>
      <vu-meter property="gain"/>
      <attenuator gain="1.0"/>
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

TEST(VUMeterTest, TestSquareWave)
{
  const string& xml = R"(
    <graph>
      <vco wave="square" freq="1"/>
      <vu-meter target="a" property="gain"/>
      <attenuator id="a" gain="0.0"/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
  ASSERT_FALSE(!fragment);

  // Should be 44100 samples at alternating -1, 1
  EXPECT_EQ(44100, fragment->waveform.size());
  for(auto i=0u; i<fragment->waveform.size(); i++)
    EXPECT_NEAR((i < 22050)?-1:1, fragment->waveform[i], 0.001)
      << i;
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  loader.load("../../sources/vco/vg-module-audio-source-vco.so");
  loader.load("../attenuator/vg-module-audio-filter-attenuator.so");
  loader.load("./vg-module-audio-filter-vu-meter.so");
  return RUN_ALL_TESTS();
}
