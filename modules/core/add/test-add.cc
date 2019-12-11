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

class AddTest: public GraphTester
{
public:
  AddTest()
  {
    loader.load("./vg-module-core-add.so");
  }
};

const auto nsamples = 100;

TEST_F(AddTest, TestSetOnlyInput)
{
  const auto expected = vector<Number>(nsamples, 42.0);
  auto actual = vector<Number>{};

  auto& ad_ = add("core/add")
              .set("input", 42.0);
  auto& snk = add_sink(actual, nsamples);
  ad_.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected, actual);
}

TEST_F(AddTest, TestSetOnlyOffset)
{
  const auto expected = vector<Number>(nsamples, 10.0);
  auto actual = vector<Number>{};

  auto& ad_ = add("core/add")
              .set("offset", 10.0);
  auto& snk = add_sink(actual, nsamples);
  ad_.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected, actual);
}

TEST_F(AddTest, TestSetBothInputAndOffset)
{
  const auto expected = vector<Number>(nsamples, 45.14);
  auto actual = vector<Number>{};

  auto& ad_ = add("core/add")
              .set("input", 42.0)
              .set("offset", 3.14);
  auto& snk = add_sink(actual, nsamples);
  ad_.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected, actual);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
