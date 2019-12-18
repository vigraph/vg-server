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

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
