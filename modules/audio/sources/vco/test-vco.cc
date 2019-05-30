//==========================================================================
// ViGraph dataflow module: audio/sources/figure/test-figure.cc
//
// Tests for <figure> source
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../audio-module-test.h"
#include "vg-geometry.h"
#include <cmath>

ModuleLoader loader;
using namespace ViGraph::Geometry;

const auto waveform_size = 44100;
const auto half_waveform_size = waveform_size / 2;
const auto quarter_waveform_size = waveform_size / 4;
const auto three_quarter_waveform_size = 3 * waveform_size / 4;

TEST(VCOTest, TestNoWaveform)
{
  FragmentGraphTester tester{loader};

  tester.add("vco");

  tester.run();

  const auto fragment = tester.get_fragment();
  ASSERT_FALSE(!fragment);

  ASSERT_EQ(1, fragment->waveforms.size());
  const auto& waveform = fragment->waveforms[Speaker::front_center];

  // Should be 44100 samples at 0
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_EQ(0.0, waveform[i]);
}

TEST(VCOTest, TestSquareWaveSingleCycle)
{
  FragmentGraphTester tester{loader};

  tester.add("vco").set("wave", "square").set("freq", 1);

  tester.run(2);

  const auto fragment = tester.get_fragment();
  ASSERT_FALSE(!fragment);

  ASSERT_EQ(1, fragment->waveforms.size());
  const auto& waveform = fragment->waveforms[Speaker::front_center];

  // Should be 44100 samples at alternating -1, 1
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
  {
    if (i < half_waveform_size)
      EXPECT_EQ(1, waveform[i]) << i;
    else
      EXPECT_EQ(-1, waveform[i]) << i;
  }
}

TEST(VCOTest, TestSquareWaveMultiCycle)
{
  FragmentGraphTester tester{loader};

  tester.add("vco").set("wave", "square").set("freq", 10);

  tester.run(2);

  const auto fragment = tester.get_fragment();
  ASSERT_FALSE(!fragment);

  ASSERT_EQ(1, fragment->waveforms.size());
  const auto& waveform = fragment->waveforms[Speaker::front_center];

  // Should be 44100 samples at alternating 1, -1
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
  {
    if (i % (waveform_size / 10) < (half_waveform_size / 10))
      EXPECT_EQ(1, waveform[i]) << i;
    else
      EXPECT_EQ(-1, waveform[i]) << i;
  }
}

TEST(VCOTest, TestSawWaveSingleCycle)
{
  FragmentGraphTester tester{loader};

  tester.add("vco").set("wave", "saw").set("freq", 1);

  tester.run(2);

  const auto fragment = tester.get_fragment();
  ASSERT_FALSE(!fragment);

  ASSERT_EQ(1, fragment->waveforms.size());
  const auto& waveform = fragment->waveforms[Speaker::front_center];

  // Should be 44100 samples linearly increasing
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
  {
    if (i < half_waveform_size)
      EXPECT_NEAR((double)i / half_waveform_size, waveform[i], 0.0001) << i;
    else
      EXPECT_NEAR((double)(i - half_waveform_size) / half_waveform_size - 1,
                  waveform[i], 0.0001) << i;
  }
}

TEST(VCOTest, TestTriangleWaveSingleCycle)
{
  FragmentGraphTester tester{loader};

  tester.add("vco").set("wave", "triangle").set("freq", 1);

  tester.run(2);

  const auto fragment = tester.get_fragment();
  ASSERT_FALSE(!fragment);

  ASSERT_EQ(1, fragment->waveforms.size());
  const auto& waveform = fragment->waveforms[Speaker::front_center];

  // Should be 44100 samples linearly increasing up and down
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
  {
    if (i < quarter_waveform_size)
      EXPECT_NEAR((double)i / quarter_waveform_size, waveform[i], 0.0001) << i;
    else if (i < half_waveform_size)
      EXPECT_NEAR(1.0 - ((double)i - quarter_waveform_size)
                        / quarter_waveform_size,
                  waveform[i], 0.0001) << i;
    else if (i < three_quarter_waveform_size)
      EXPECT_NEAR(-((double)i - half_waveform_size) / quarter_waveform_size,
                  waveform[i], 0.0001) << i;
    else
      EXPECT_NEAR(-1.0 + ((double)i - three_quarter_waveform_size)
                         / quarter_waveform_size,
                  waveform[i], 0.0001) << i;
  }
}

TEST(VCOTest, TestSinWaveSingleCycle)
{
  FragmentGraphTester tester{loader};

  tester.add("vco").set("wave", "sin").set("freq", 1);

  tester.run(2);

  const auto fragment = tester.get_fragment();
  ASSERT_FALSE(!fragment);

  ASSERT_EQ(1, fragment->waveforms.size());
  const auto& waveform = fragment->waveforms[Speaker::front_center];

  // Should be 44100 samples in sin -1..1
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_NEAR(sin(2*pi*(double)i/waveform.size()), waveform[i], 0.0001) << i;
}

TEST(VCOTest, TestRandomSingleCycle)
{
  FragmentGraphTester tester{loader};

  tester.add("vco").set("wave", "random").set("freq", 1);

  tester.run(2);

  const auto fragment = tester.get_fragment();
  ASSERT_FALSE(!fragment);

  ASSERT_EQ(1, fragment->waveforms.size());
  const auto& waveform = fragment->waveforms[Speaker::front_center];

  // Should be 44100 samples random between -1 and 1
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
  {
    EXPECT_GE(1.0, waveform[i]) << i;
    EXPECT_LE(-1.0, waveform[i]) << i;
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  loader.load("./vg-module-audio-source-vco.so");
  loader.add_default_section("audio");
  return RUN_ALL_TESTS();
}
