//==========================================================================
// ViGraph dataflow module: core/subtract/test-subtract.cc
//
// Tests for <subtract> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
#include "vg-waveform.h"
#include <cmath>

class SubtractTest: public GraphTester
{
public:
  SubtractTest()
  {
    loader.load("./vg-module-core-subtract.so");
  }
};

const auto nsamples = 100;

TEST_F(SubtractTest, TestSetOnlyInput)
{
  const auto expected = vector<Number>(nsamples, 42.0);
  auto actual = vector<Number>{};

  auto& sub = add("core/subtract")
              .set("input", 42.0);
  auto& snk = add_sink(actual, nsamples);
  sub.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected, actual);
}

TEST_F(SubtractTest, TestSetOnlyOffset)
{
  const auto expected = vector<Number>(nsamples, -10.0);
  auto actual = vector<Number>{};

  auto& sub = add("core/subtract")
              .set("offset", 10.0);
  auto& snk = add_sink(actual, nsamples);
  sub.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected, actual);
}

TEST_F(SubtractTest, TestSetBothInputAndOffset)
{
  const auto expected = vector<Number>(nsamples, 38.86);
  auto actual = vector<Number>{};

  auto& sub = add("core/subtract")
              .set("input", 42.0)
              .set("offset", 3.14);
  auto& snk = add_sink(actual, nsamples);
  sub.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected, actual);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
