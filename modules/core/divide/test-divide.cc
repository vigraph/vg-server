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

class DivideTest: public GraphTester
{
public:
  DivideTest()
  {
    loader.load("./vg-module-core-divide.so");
  }
};

const auto nsamples = 100;

TEST_F(DivideTest, TestSetOnlyInput)
{
  const auto expected = vector<Number>(nsamples, 42.0);
  auto actual = vector<Number>{};

  auto& dvd = add("core/divide")
              .set("input", 42.0);
  auto& snk = add_sink(actual, nsamples);
  dvd.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected, actual);
}

TEST_F(DivideTest, TestSetOnlyFactor)
{
  const auto expected = vector<Number>(nsamples, 0.0);
  auto actual = vector<Number>{};

  auto& dvd = add("core/divide")
              .set("factor", 10.0);
  auto& snk = add_sink(actual, nsamples);
  dvd.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected, actual);
}

TEST_F(DivideTest, TestSetBothInputAndFactor)
{
  const auto expected = vector<Number>(nsamples, 4.2);
  auto actual = vector<Number>{};

  auto& dvd = add("core/divide")
              .set("input", 42.0)
              .set("factor", 10.0);
  auto& snk = add_sink(actual, nsamples);
  dvd.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected, actual);
}

TEST_F(DivideTest, TestSetDivideByZeroPositive)
{
  const auto expected = vector<Number>(nsamples, DBL_MAX);
  auto actual = vector<Number>{};

  auto& dvd = add("core/divide")
              .set("input", 42.0)
              .set("factor", 0.0);
  auto& snk = add_sink(actual, nsamples);
  dvd.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected, actual);
}

TEST_F(DivideTest, TestSetDivideByZeroNegative)
{
  const auto expected = vector<Number>(nsamples, DBL_MIN);
  auto actual = vector<Number>{};

  auto& dvd = add("core/divide")
              .set("input", -42.0)
              .set("factor", 0.0);
  auto& snk = add_sink(actual, nsamples);
  dvd.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected, actual);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
