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

class MultiplyTest: public GraphTester
{
public:
  MultiplyTest()
  {
    loader.load("./vg-module-core-multiply.so");
  }
};

const auto nsamples = 100;

TEST_F(MultiplyTest, TestSetOnlyInput)
{
  const auto expected = vector<Number>(nsamples, 42.0);
  auto actual = vector<Number>{};

  auto& mlt = add("core/multiply")
              .set("input", 42.0);
  auto& snk = add_sink(actual, nsamples);
  mlt.connect("output", snk, "input");


  run();

  EXPECT_EQ(expected, actual);
}

TEST_F(MultiplyTest, TestSetOnlyFactor)
{
  const auto expected = vector<Number>(nsamples, 0.0);
  auto actual = vector<Number>{};

  auto& mlt = add("core/multiply")
              .set("factor", 10.0);
  auto& snk = add_sink(actual, nsamples);
  mlt.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected, actual);
}

TEST_F(MultiplyTest, TestSetBothInputAndFactor)
{
  const auto expected = vector<Number>(nsamples, 4.2);
  auto actual = vector<Number>{};

  auto& mlt = add("core/multiply")
              .set("input", 42.0)
              .set("factor", 0.1);
  auto& snk = add_sink(actual, nsamples);
  mlt.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected, actual);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
