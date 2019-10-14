//==========================================================================
// ViGraph dataflow module: core/oscillator/test-oscillator.cc
//
// Tests for <oscillator> source
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
#include "vg-geometry.h"
#include "vg-waveform.h"
#include <cmath>

ModuleLoader loader;
using namespace ViGraph::Geometry;

const auto waveform_size = 44100;
const auto half_waveform_size = waveform_size / 2;
const auto quarter_waveform_size = waveform_size / 4;
const auto three_quarter_waveform_size = 3 * waveform_size / 4;

TEST(OscillatorTest, TestNoWaveform)
{
  GraphTester<double> tester{loader, waveform_size};

  auto& osc = tester.add("oscillator");
  tester.capture_from(osc, "output");

  tester.run();

  const auto waveform = tester.get_output();

  // Should be 44100 samples at 0
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_EQ(0.0, waveform[i]);
}

TEST(OscillatorTest, TestSquareWaveSingleCycle)
{
  GraphTester<double> tester{loader, waveform_size};

  auto& osc = tester.add("oscillator")
                    .set("wave", Waveform::Type::square)
                    .set("freq", 1.0);
  tester.capture_from(osc, "output");

  tester.run();

  const auto waveform = tester.get_output();

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

TEST(OscillatorTest, TestSquareWaveMultiCycle)
{
  GraphTester<double> tester{loader, waveform_size};

  auto& osc = tester.add("oscillator")
                    .set("wave", Waveform::Type::square)
                    .set("freq", 10.0);
  tester.capture_from(osc, "output");

  tester.run();

  const auto waveform = tester.get_output();

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

TEST(OscillatorTest, TestSawWaveSingleCycle)
{
  GraphTester<double> tester{loader, waveform_size};

  auto& osc = tester.add("oscillator")
                    .set("wave", Waveform::Type::saw)
                    .set("freq", 1.0);
  tester.capture_from(osc, "output");

  tester.run(2);

  const auto waveform = tester.get_output();

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

TEST(OscillatorTest, TestTriangleWaveSingleCycle)
{
  GraphTester<double> tester{loader, waveform_size};

  auto& osc = tester.add("oscillator")
                    .set("wave", Waveform::Type::triangle)
                    .set("freq", 1.0);
  tester.capture_from(osc, "output");

  tester.run();

  const auto waveform = tester.get_output();

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

TEST(OscillatorTest, TestSinWaveSingleCycle)
{
  GraphTester<double> tester{loader, waveform_size};

  auto& osc = tester.add("oscillator")
                    .set("wave", Waveform::Type::sin)
                    .set("freq", 1.0);
  tester.capture_from(osc, "output");

  tester.run();

  const auto waveform = tester.get_output();

  // Should be 44100 samples in sin -1..1
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_NEAR(sin(2*pi*(double)i/waveform.size()), waveform[i], 0.0001) << i;
}

TEST(OscillatorTest, TestRandomSingleCycle)
{
  GraphTester<double> tester{loader, waveform_size};

  auto& osc = tester.add("oscillator")
                    .set("wave", Waveform::Type::random)
                    .set("freq", 1.0);
  tester.capture_from(osc, "output");

  tester.run();

  const auto waveform = tester.get_output();

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
  loader.load("./vg-module-core-oscillator.so");
  return RUN_ALL_TESTS();
}
