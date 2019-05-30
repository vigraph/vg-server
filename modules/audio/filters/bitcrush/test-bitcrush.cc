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
  FragmentGraphTester tester{loader};

  auto vco = tester.add("vco");
  auto bitcrush = tester.add("bitcrush");

  vco.connect("default", bitcrush, "default");

  tester.run(2);

  const auto fragment = tester.get_fragment();
  ASSERT_FALSE(!fragment);

  const auto& waveform = fragment->waveforms[Speaker::front_center];

  // Should be 44100 samples at 0
  EXPECT_EQ(44100, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_NEAR(0.0, waveform[i], 0.001);
}

TEST(BitCrushTest, Test2BitSinWave)
{
  FragmentGraphTester tester{loader};

  auto vco = tester.add("vco").set("wave", "sin").set("freq", 1);
  auto bitcrush = tester.add("bitcrush").set("bits", 2);

  vco.connect("default", bitcrush, "default");

  tester.run(2);

  const auto fragment = tester.get_fragment();
  ASSERT_FALSE(!fragment);

  const auto& waveform = fragment->waveforms[Speaker::front_center];

  // Should be 44100 samples at 4 quantised values
  EXPECT_EQ(44100, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_NEAR((i < 5122) ? 0.333
                : (i < 16929) ? 1.0
                : (i < 22050) ? 0.333
                : (i < 27172) ? -0.333
                : (i < 38979) ? -1.0
                : -0.333,
                waveform[i], 0.001) << i;
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  loader.load("../../sources/vco/vg-module-audio-source-vco.so");
  loader.load("./vg-module-audio-filter-bitcrush.so");
  loader.add_default_section("audio");
  return RUN_ALL_TESTS();
}
