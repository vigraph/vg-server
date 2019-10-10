//==========================================================================
// ViGraph dataflow module: core/add/test-add.cc
//
// Tests for <add> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
#include "vg-waveform.h"
#include <cmath>

ModuleLoader loader;

const auto waveform_size = 44100;

TEST(AddTest, TestSetOnlyInput)
{
  GraphTester<double> tester{loader, waveform_size};

  auto& osc = tester.add("add")
                    .set("input", 42.0);
  tester.capture_from(osc, "output");

  tester.run();

  const auto waveform = tester.get_output();

  // Should be 44100 samples at 42
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_DOUBLE_EQ(42.0, waveform[i]);
}

TEST(AddTest, TestSetOnlyOffset)
{
  GraphTester<double> tester{loader, waveform_size};

  auto& osc = tester.add("add")
                    .set("offset", 10.0);
  tester.capture_from(osc, "output");

  tester.run();

  const auto waveform = tester.get_output();

  // Should be 44100 samples at 10.0
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_DOUBLE_EQ(10.0, waveform[i]);
}

TEST(AddTest, TestSetBothInputAndOffset)
{
  GraphTester<double> tester{loader, waveform_size};

  auto& osc = tester.add("add")
                    .set("input", 42.0)
                    .set("offset", 3.14);
  tester.capture_from(osc, "output");

  tester.run();

  const auto waveform = tester.get_output();

  // Should be 44100 samples at 45.14
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_DOUBLE_EQ(45.14, waveform[i]);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  loader.load("./vg-module-core-add.so");
  return RUN_ALL_TESTS();
}
