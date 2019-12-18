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

class OscillatorTest: public GraphTester
{
public:
  OscillatorTest()
  {
    loader.load("./vg-module-core-oscillator.so");
  }
};


using namespace ViGraph::Geometry;

const auto waveform_size = 44100;
const auto half_waveform_size = waveform_size / 2;
const auto quarter_waveform_size = waveform_size / 4;
const auto three_quarter_waveform_size = 3 * waveform_size / 4;

TEST_F(OscillatorTest, TestNoWaveform)
{
  auto& osc = add("core/oscillator");
  auto waveform = vector<Number>{};
  auto& snk = add_sink(waveform, waveform_size);
  osc.connect("output", snk, "input");

  run();

  // Should be 44100 samples at 0.5
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_EQ(0.5, waveform[i]);
}

TEST_F(OscillatorTest, TestSquareWaveSingleCycle)
{
  auto& osc = add("core/oscillator")
              .set("wave", Waveform::Type::square)
              .set("period", 1.0);
  auto waveform = vector<Number>{};
  auto& snk = add_sink(waveform, waveform_size);
  osc.connect("output", snk, "input");

  run();

  // Should be 44100 samples at alternating 1, 0
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
  {
    if (i < half_waveform_size)
      EXPECT_EQ(1, waveform[i]) << i;
    else
      EXPECT_EQ(0, waveform[i]) << i;
  }
}

TEST_F(OscillatorTest, TestSquareWaveStartConnectedButNotStarted)
{
  auto& osc = add("core/oscillator")
              .set("wave", Waveform::Type::square)
              .set("period", 1.0);
  auto start_data = vector<Trigger>(100);
  auto& sts = add_source(start_data);
  sts.connect("output", osc, "start");
  auto waveform = vector<Number>{};
  auto& snk = add_sink(waveform, waveform_size);
  osc.connect("output", snk, "input");

  run();

  // Should be 44100 samples at 0
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_EQ(0.0, waveform[i]) << i;
}

TEST_F(OscillatorTest, TestSquareWaveStartHalfWayThrough)
{
  auto& osc = add("core/oscillator")
              .set("wave", Waveform::Type::square)
              .set("period", 0.5);

  auto start_data = vector<Trigger>(waveform_size);
  start_data[half_waveform_size] = 1;
  auto& sts = add_source(start_data);
  sts.connect("output", osc, "start");
  auto waveform = vector<Number>{};
  auto& snk = add_sink(waveform, waveform_size);
  osc.connect("output", snk, "input");

  run();

  // Should be 44100 samples, half at 0, half at alternating 1.0, 0
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
  {
    if (i < half_waveform_size)
      EXPECT_EQ(0.0, waveform[i]) << i;
    else if (i < three_quarter_waveform_size)
      EXPECT_EQ(1, waveform[i]) << i;
    else
      EXPECT_EQ(0, waveform[i]) << i;
  }
}

TEST_F(OscillatorTest, TestSquareWaveStopPartWayThrough)
{
  auto& osc = add("core/oscillator")
              .set("wave", Waveform::Type::square)
              .set("period", 0.5);

  // Note we have to connect and trigger start otherwise it will
  // retrigger automatically because not connected
  auto start_data = vector<Trigger>(waveform_size);
  start_data[0] = 1;
  auto& sts = add_source(start_data);
  sts.connect("output", osc, "start");

  auto stop_data = vector<Trigger>(waveform_size);
  // Note we set the stop half-way through the waveform, to test it is
  // properly run out to theta wrap
  stop_data[quarter_waveform_size] = 1;
  auto& sos = add_source(stop_data);
  sos.connect("output", osc, "stop");
  auto waveform = vector<Number>{};
  auto& snk = add_sink(waveform, waveform_size);
  osc.connect("output", snk, "input");

  run();

  // Should be 44100 samples, half at 0, half at alternating 1.0, 0
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
  {
    if (i < quarter_waveform_size)
      EXPECT_EQ(1, waveform[i]) << i;
    else if (i < half_waveform_size)
      EXPECT_EQ(0, waveform[i]) << i;
    else
      EXPECT_EQ(0.0, waveform[i]) << i;
  }
}

TEST_F(OscillatorTest, TestSquareWaveMultiCycle)
{
  auto& osc = add("core/oscillator")
              .set("wave", Waveform::Type::square)
              .set("period", 0.1);
  auto waveform = vector<Number>{};
  auto& snk = add_sink(waveform, waveform_size);
  osc.connect("output", snk, "input");

  run();

  // Should be 44100 samples at alternating 1, 0
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
  {
    if (i % (waveform_size / 10) < (half_waveform_size / 10))
      EXPECT_EQ(1, waveform[i]) << i;
    else
      EXPECT_EQ(0, waveform[i]) << i;
  }
}

TEST_F(OscillatorTest, TestSawWaveSingleCycle)
{
  auto& osc = add("core/oscillator")
              .set("wave", Waveform::Type::saw)
              .set("period", 1.0);
  auto waveform = vector<Number>{};
  auto& snk = add_sink(waveform, waveform_size);
  osc.connect("output", snk, "input");

  run(2);

  // Should be 44100 samples linearly increasing
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
  {
    if (i < half_waveform_size)
      EXPECT_NEAR(0.5+(double)i / waveform_size, waveform[i], 0.0001) << i;
    else
      EXPECT_NEAR((double)(i - half_waveform_size) / waveform_size,
                  waveform[i], 0.0001) << i;
  }
}

TEST_F(OscillatorTest, TestTriangleWaveSingleCycle)
{
  auto& osc = add("core/oscillator")
              .set("wave", Waveform::Type::triangle)
              .set("period", 1.0);
  auto waveform = vector<Number>{};
  auto& snk = add_sink(waveform, waveform_size);
  osc.connect("output", snk, "input");

  run();

  // Should be 44100 samples linearly increasing up and down
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
  {
    if (i < quarter_waveform_size)
      EXPECT_NEAR(0.5 + (double)i / half_waveform_size, waveform[i],
                  0.0001) << i;
    else if (i < three_quarter_waveform_size)
      EXPECT_NEAR(1.0- ((double)i - quarter_waveform_size) / half_waveform_size,
                  waveform[i], 0.0001) << i;
    else
      EXPECT_NEAR(((double)i - three_quarter_waveform_size)
                  / half_waveform_size,
                  waveform[i], 0.0001) << i;
  }
}

TEST_F(OscillatorTest, TestSinWaveSingleCycle)
{
  auto& osc = add("core/oscillator")
              .set("wave", Waveform::Type::sin)
              .set("period", 1.0);
  auto waveform = vector<Number>{};
  auto& snk = add_sink(waveform, waveform_size);
  osc.connect("output", snk, "input");

  run();

  // Should be 44100 samples in sin 0..1
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_NEAR(sin(2*pi*(double)i/waveform.size())/2+0.5,
                waveform[i], 0.0001) << i;
}

TEST_F(OscillatorTest, TestRandomSingleCycle)
{
  auto& osc = add("core/oscillator")
             .set("wave", Waveform::Type::random);
  auto waveform = vector<Number>{};
  auto& snk = add_sink(waveform, waveform_size);
  osc.connect("output", snk, "input");

  run();

  // Should be 44100 samples random between 0 and 1
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
  {
    EXPECT_GE(1.0, waveform[i]) << i;
    EXPECT_LE(0.0, waveform[i]) << i;
  }
}

TEST_F(OscillatorTest, TestSquareWaveSingleCycleWithPhase)
{
  auto& osc = add("core/oscillator")
              .set("wave", Waveform::Type::square)
              .set("period", 1.0)
              .set("phase", 0.25);
  auto waveform = vector<Number>{};
  auto& snk = add_sink(waveform, waveform_size);
  osc.connect("output", snk, "input");

  run();

  // Should be 44100 samples at alternating 0, 1, starting at 1/4
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
  {
    if (i < quarter_waveform_size || i >= three_quarter_waveform_size)
      EXPECT_EQ(1, waveform[i]) << i;
    else
      EXPECT_EQ(0, waveform[i]) << i;
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
