//==========================================================================
// ViGraph dataflow module: core/oscillator/test-oscillator.cc
//
// Tests for <oscillator> source
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
#include "vg-waveform.h"
#include <cmath>

class OscillatorTest: public GraphTester
{
public:
  OscillatorTest()
  {
    loader.load("./vg-module-audio-oscillator.so");
  }
};


const auto waveform_size = 44100;

TEST_F(OscillatorTest, TestNoWaveform)
{
  auto& osc = add("audio/oscillator");
  auto waveform = vector<Number>{};
  auto& snk = add_sink(waveform, waveform_size);
  osc.connect("output", snk, "input");

  run();

  // Should be 44100 samples at 0
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_EQ(0.0, waveform[i]);
}

TEST_F(OscillatorTest, TestDefaultSquareWaveFrequency)
{
  auto& osc = add("audio/oscillator")
             .set("wave", Waveform::Type::square);
  auto waveform = vector<Number>{};
  auto& snk = add_sink(waveform, waveform_size);
  osc.connect("output", snk, "input");

  run();

  // Should be 44100 samples with 262 rising edges
  EXPECT_EQ(waveform_size, waveform.size());
  auto last = 0.0;
  auto rising = 0;
  auto high = 0;
  for(auto i=0u; i<waveform.size(); i++)
  {
    auto v = waveform[i];
    if (last < 0 && v >= 0) rising++;
    if (v >= 0) high++;
    last = v;

    EXPECT_TRUE(v==-1.0 || v==1.0);
  }

  EXPECT_NEAR(262, rising, 1);
  EXPECT_NEAR(22050, high, 50);  // Roughly half the time
}

TEST_F(OscillatorTest, TestDefaultSinWaveFrequency)
{
  auto& osc = add("audio/oscillator")
             .set("wave", Waveform::Type::sin);
  auto waveform = vector<Number>{};
  auto& snk = add_sink(waveform, waveform_size);
  osc.connect("output", snk, "input");

  run();

  // Should be 44100 samples with 262 rising edges
  EXPECT_EQ(waveform_size, waveform.size());
  auto last = 0.0;
  auto rising = 0;
  for(auto i=0u; i<waveform.size(); i++)
  {
    auto v = waveform[i];
    if (last < 0 && v >= 0) rising++;
    last = v;

    EXPECT_LE(-1.0, v);
    EXPECT_GE(1.0, v);
  }

  EXPECT_NEAR(262, rising, 1);
}

TEST_F(OscillatorTest, TestA440SquareWaveFrequency)
{
  auto& osc = add("audio/oscillator")
             .set("wave", Waveform::Type::square)
             .set("note", 0.75);
  auto waveform = vector<Number>{};
  auto& snk = add_sink(waveform, waveform_size);
  osc.connect("output", snk, "input");

  run();

  // Should be 44100 samples with 439 rising edges
  EXPECT_EQ(waveform_size, waveform.size());
  auto last = 0.0;
  auto rising = 0;
  for(auto i=0u; i<waveform.size(); i++)
  {
    auto v = waveform[i];
    if (last < 0 && v >= 0) rising++;
    last = v;

    EXPECT_TRUE(v==-1.0 || v==1.0);
  }

  EXPECT_NEAR(440, rising, 1);
}

TEST_F(OscillatorTest, TestUpAnOctaveSquareWaveFrequency)
{
  auto& osc = add("audio/oscillator")
             .set("wave", Waveform::Type::square)
             .set("octave", 1.0);
  auto waveform = vector<Number>{};
  auto& snk = add_sink(waveform, waveform_size);
  osc.connect("output", snk, "input");

  run();

  // Should be 44100 samples with 523 rising edges
  EXPECT_EQ(waveform_size, waveform.size());
  auto last = 0.0;
  auto rising = 0;
  for(auto i=0u; i<waveform.size(); i++)
  {
    auto v = waveform[i];
    if (last < 0 && v >= 0) rising++;
    last = v;

    EXPECT_TRUE(v==-1.0 || v==1.0);
  }

  EXPECT_NEAR(523, rising, 1);
}

TEST_F(OscillatorTest, TestDetunedSquareWaveFrequency)
{
  auto& osc = add("audio/oscillator")
             .set("wave", Waveform::Type::square)
             .set("detune", 12.0);
  auto waveform = vector<Number>{};
  auto& snk = add_sink(waveform, waveform_size);
  osc.connect("output", snk, "input");

  run();

  // Should be 44100 samples with 523 rising edges
  EXPECT_EQ(waveform_size, waveform.size());
  auto last = 0.0;
  auto rising = 0;
  for(auto i=0u; i<waveform.size(); i++)
  {
    auto v = waveform[i];
    if (last < 0 && v >= 0) rising++;
    last = v;

    EXPECT_TRUE(v==-1.0 || v==1.0);
  }

  EXPECT_NEAR(523, rising, 1);
}

TEST_F(OscillatorTest, Test25PercentPulseWidthSquareWave)
{
  auto& osc = add("audio/oscillator")
             .set("wave", Waveform::Type::square)
             .set("pulse-width", 0.25);
  auto waveform = vector<Number>{};
  auto& snk = add_sink(waveform, waveform_size);
  osc.connect("output", snk, "input");

  run();

  // Should be 44100 samples with 262 rising edges
  EXPECT_EQ(waveform_size, waveform.size());
  auto last = 0.0;
  auto rising = 0;
  auto high = 0;
  for(auto i=0u; i<waveform.size(); i++)
  {
    auto v = waveform[i];
    if (last < 0 && v >= 0) rising++;
    if (v >= 0) high++;
    last = v;

    EXPECT_TRUE(v==-1.0 || v==1.0);
  }

  EXPECT_NEAR(262, rising, 1);
  EXPECT_NEAR(11025, high, 50);  // Roughly quarter the time
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
