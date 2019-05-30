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
  FragmentGraphTester tester{loader};

  auto vco = tester.add("vco");
  auto attenuator = tester.add("attenuator").set("gain", 0.2);

  vco.connect("default", attenuator, "default");

  tester.run(2);

  const auto fragment = tester.get_fragment();
  ASSERT_FALSE(!fragment);

  const auto& waveform = fragment->waveforms[Speaker::front_center];

  // Should be 44100 samples at 0
  EXPECT_EQ(44100, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_EQ(0.0, waveform[i]);
}

TEST(AttenuatorTest, TestSquareWaveReduced)
{
  FragmentGraphTester tester{loader};

  auto vco = tester.add("vco").set("wave", "square").set("freq", 1);
  auto attenuator = tester.add("attenuator").set("gain", 0.2);

  vco.connect("default", attenuator, "default");

  tester.run(2);

  const auto fragment = tester.get_fragment();
  ASSERT_FALSE(!fragment);

  const auto& waveform = fragment->waveforms[Speaker::front_center];

  // Should be 44100 samples at alternating -1, 1
  EXPECT_EQ(44100, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_NEAR((i < 22050)?0.2:-0.2, waveform[i], 0.001)
      << i;
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  loader.load("../../sources/vco/vg-module-audio-source-vco.so");
  loader.load("./vg-module-audio-filter-attenuator.so");
  loader.add_default_section("audio");
  return RUN_ALL_TESTS();
}
