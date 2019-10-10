//==========================================================================
// ViGraph dataflow module: core/multiply/test-multiply.cc
//
// Tests for <multiply> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
#include "vg-waveform.h"
#include <cmath>

ModuleLoader loader;

const auto waveform_size = 44100;

TEST(MultiplyTest, TestSetOnlyInput)
{
  GraphTester<double> tester{loader, waveform_size};

  auto& osc = tester.add("multiply")
                    .set("input", 42.0);
  tester.capture_from(osc, "output");

  tester.run();

  const auto waveform = tester.get_output();

  // Should be 44100 samples at 42
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_DOUBLE_EQ(42.0, waveform[i]);
}

TEST(MultiplyTest, TestSetOnlyFactor)
{
  GraphTester<double> tester{loader, waveform_size};

  auto& osc = tester.add("multiply")
                    .set("factor", 10.0);
  tester.capture_from(osc, "output");

  tester.run();

  const auto waveform = tester.get_output();

  // Should be 44100 samples at 0.0
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_DOUBLE_EQ(0.0, waveform[i]);
}

TEST(MultiplyTest, TestSetBothInputAndFactor)
{
  GraphTester<double> tester{loader, waveform_size};

  auto& osc = tester.add("multiply")
                    .set("input", 42.0)
                    .set("factor", 0.1);
  tester.capture_from(osc, "output");

  tester.run();

  const auto waveform = tester.get_output();

  // Should be 44100 samples at 4.2
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_DOUBLE_EQ(4.2, waveform[i]);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  loader.load("./vg-module-core-multiply.so");
  return RUN_ALL_TESTS();
}
