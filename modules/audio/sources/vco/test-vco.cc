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

TEST(VCOTest, TestNoWaveform)
{
  const string& xml = R"(
    <graph>
      <vco/>
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

TEST(VCOTest, TestSquareWaveSingleCycle)
{
  const string& xml = R"(
    <graph>
      <vco wave="square" freq="1"/>
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

TEST(VCOTest, TestSquareWaveMultiCycle)
{
  const string& xml = R"(
    <graph>
      <vco wave="square" freq="10"/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
  ASSERT_FALSE(!fragment);

  // Should be 44100 samples at alternating -1, 1
  EXPECT_EQ(44100, fragment->waveform.size());
  for(auto i=0u; i<fragment->waveform.size(); i++)
    EXPECT_EQ((i%4410 < 2205)?-1:1, fragment->waveform[i]) << i;
}

TEST(VCOTest, TestSawWaveSingleCycle)
{
  const string& xml = R"(
    <graph>
      <vco wave="saw" freq="1"/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
  ASSERT_FALSE(!fragment);

  // Should be 44100 samples linearly increasing
  EXPECT_EQ(44100, fragment->waveform.size());
  for(auto i=0u; i<fragment->waveform.size(); i++)
    EXPECT_NEAR((double)i/fragment->waveform.size()*2-1,
                fragment->waveform[i], 0.0001) << i;
}

TEST(VCOTest, TestTriangleWaveSingleCycle)
{
  const string& xml = R"(
    <graph>
      <vco wave="triangle" freq="1"/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
  ASSERT_FALSE(!fragment);

  // Should be 44100 samples linearly increasing up and down
  EXPECT_EQ(44100, fragment->waveform.size());
  for(auto i=0u; i<fragment->waveform.size(); i++)
    EXPECT_NEAR(i<22050?(double)i/fragment->waveform.size()*4-1
                       :(1-(double)i/fragment->waveform.size())*4-1,
                fragment->waveform[i], 0.0001) << i;
}

TEST(VCOTest, TestSinWaveSingleCycle)
{
  const string& xml = R"(
    <graph>
      <vco wave="sin" freq="1"/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
  ASSERT_FALSE(!fragment);

  // Should be 44100 samples in sin -1..1
  EXPECT_EQ(44100, fragment->waveform.size());
  for(auto i=0u; i<fragment->waveform.size(); i++)
    EXPECT_NEAR(sin(2*pi*(double)i/fragment->waveform.size()),
                fragment->waveform[i], 0.0001) << i;
}

TEST(VCOTest, TestRandomSingleCycle)
{
  const string& xml = R"(
    <graph>
      <vco wave="random" freq="1"/>
    </graph>
  )";

  FragmentGenerator gen(xml, loader, 2);  // 1 to start, 1 to generate (at 1.0)
  Fragment *fragment = gen.get_fragment();
  ASSERT_FALSE(!fragment);

  // Should be 44100 samples random between -1 and 1
  EXPECT_EQ(44100, fragment->waveform.size());
  for(auto i=0u; i<fragment->waveform.size(); i++)
  {
    EXPECT_GE(1.0, fragment->waveform[i]) << i;
    EXPECT_LE(-1.0,  fragment->waveform[i]) << i;
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  loader.load("./vg-module-audio-source-vco.so");
  return RUN_ALL_TESTS();
}
