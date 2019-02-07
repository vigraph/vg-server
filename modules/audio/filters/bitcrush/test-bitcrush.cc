//==========================================================================
// ViGraph dataflow module: audio/sources/bitcrush/test-bitcrush.cc
//
// Tests for <bitcrush> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module-test.h"
#include "vg-geometry.h"
#include <cmath>

ModuleLoader loader;
using namespace ViGraph::Geometry;

TEST(BitCrushTest, TestNoWaveform)
{
  const string& xml = R"(
    <graph>
      <vco/>
      <bitcrush/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
  ASSERT_FALSE(!fragment);

  // Should be 44100 samples at 0
  EXPECT_EQ(44100, fragment->waveform.size());
  for(auto i=0u; i<fragment->waveform.size(); i++)
    EXPECT_NEAR(0.0, fragment->waveform[i], 0.001);
}

TEST(BitCrushTest, Test2BitSinWave)
{
  const string& xml = R"(
    <graph>
      <vco wave="sin" freq="1"/>
      <bitcrush bits="2"/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
  ASSERT_FALSE(!fragment);

  // Should be 44100 samples at 4 quantised values
  EXPECT_EQ(44100, fragment->waveform.size());
  for(auto i=0u; i<fragment->waveform.size(); i++)
    EXPECT_NEAR((i < 5122) ? 0.333
                : (i < 16929) ? 1.0
                : (i < 22050) ? 0.333
                : (i < 27172) ? -0.333
                : (i < 38979) ? -1.0
                : -0.333,
                fragment->waveform[i], 0.001) << i;
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  loader.load("../../sources/vco/vg-module-audio-source-vco.so");
  loader.load("./vg-module-audio-filter-bitcrush.so");
  return RUN_ALL_TESTS();
}
