//==========================================================================
// ViGraph dataflow module: core/divide/test-divide.cc
//
// Tests for <divide> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
#include "vg-waveform.h"
#include <cfloat>

ModuleLoader loader;

const auto waveform_size = 44100;

TEST(DivideTest, TestSetOnlyInput)
{
  GraphTester<double> tester{loader, waveform_size};

  auto& osc = tester.add("divide")
                    .set("input", 42.0);
  tester.capture_from(osc, "output");

  tester.run();

  const auto waveform = tester.get_output();

  // Should be 44100 samples at 42
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_DOUBLE_EQ(42.0, waveform[i]);
}

TEST(DivideTest, TestSetOnlyFactor)
{
  GraphTester<double> tester{loader, waveform_size};

  auto& osc = tester.add("divide")
                    .set("factor", 10.0);
  tester.capture_from(osc, "output");

  tester.run();

  const auto waveform = tester.get_output();

  // Should be 44100 samples at 0.0
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_DOUBLE_EQ(0.0, waveform[i]);
}

TEST(DivideTest, TestSetBothInputAndFactor)
{
  GraphTester<double> tester{loader, waveform_size};

  auto& osc = tester.add("divide")
                    .set("input", 42.0)
                    .set("factor", 10.0);
  tester.capture_from(osc, "output");

  tester.run();

  const auto waveform = tester.get_output();

  // Should be 44100 samples at 4.2
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_NEAR(4.2, waveform[i], 1e-6);
}

TEST(DivideTest, TestSetDivideByZeroPositive)
{
  GraphTester<double> tester{loader, waveform_size};

  auto& osc = tester.add("divide")
                    .set("input", 42.0)
                    .set("factor", 0.0);
  tester.capture_from(osc, "output");

  tester.run();

  const auto waveform = tester.get_output();

  // Should be 44100 samples at DBL_MAX
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_DOUBLE_EQ(DBL_MAX, waveform[i]);
}

TEST(DivideTest, TestSetDivideByZeroNegative)
{
  GraphTester<double> tester{loader, waveform_size};

  auto& osc = tester.add("divide")
                    .set("input", -42.0)
                    .set("factor", 0.0);
  tester.capture_from(osc, "output");

  tester.run();

  const auto waveform = tester.get_output();

  // Should be 44100 samples at DBL_MIN
  EXPECT_EQ(waveform_size, waveform.size());
  for(auto i=0u; i<waveform.size(); i++)
    EXPECT_DOUBLE_EQ(DBL_MIN, waveform[i]);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  loader.load("./vg-module-core-divide.so");
  return RUN_ALL_TESTS();
}
