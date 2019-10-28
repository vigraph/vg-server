//==========================================================================
// ViGraph dataflow module: core/average/test-average.cc
//
// Tests for <average> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
#include "vg-waveform.h"
#include <cmath>

class AverageTest: public GraphTester
{
public:
  AverageTest()
  {
    loader.load("./vg-module-core-average.so");
  }
};

const auto samples = 10;

TEST_F(AverageTest, TestAverage)
{
  auto& avg = add("average")
              .set("samples", samples)
              .set("input", 42.0);
  auto output = vector<double>{};
  auto& snk = add_sink(output, samples);
  avg.connect("output", snk, "input");

  run();

  // Should be 10 samples at 42
  EXPECT_EQ(samples, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_DOUBLE_EQ(42.0, output[i]);
}

TEST_F(AverageTest, TestRMSAverage)
{
  auto& avg = add("average")
              .set("samples", samples)
              .set("rms", true)
              .set("input", 42.0);
  auto output = vector<double>{};
  auto& snk = add_sink(output, samples);
  avg.connect("output", snk, "input");

  run();

  // Should be 10 samples at 42
  EXPECT_EQ(samples, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_DOUBLE_EQ(42.0, output[i]);
}

TEST_F(AverageTest, TestAverageForVariantData)
{
  auto input_data = vector<double>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  auto expected = vector<double>{1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5};
  auto& src = add_source(input_data);
  auto& avg = add("average")
              .set("samples", samples);
  src.connect("output", avg, "input");
  auto output = vector<double>{};
  auto& snk = add_sink(output, samples);
  avg.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected, output);
}

TEST_F(AverageTest, TestRMSAverageForVariantData)
{
  auto input_data = vector<double>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  auto expected = vector<double>{
    1,
    1.5811388300841898,
    2.1602468994692869,
    2.7386127875258306,
    3.3166247903553998,
    3.8944404818493075,
    4.4721359549995796,
    5.0497524691810387,
    5.6273143387113773,
    6.2048368229954285
  };
  auto& src = add_source(input_data);
  auto& avg = add("average")
              .set("samples", samples)
              .set("rms", true);
  src.connect("output", avg, "input");
  auto output = vector<double>{};
  auto& snk = add_sink(output, samples);
  avg.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected.size(), output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_DOUBLE_EQ(expected[i], output[i]);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
