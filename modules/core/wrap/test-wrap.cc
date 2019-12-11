//==========================================================================
// ViGraph dataflow module: core/wrap/test-wrap.cc
//
// Tests for wrap control
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class WrapTest: public GraphTester
{
public:
  WrapTest()
  {
    loader.load("./vg-module-core-wrap.so");
  }
};

const auto sample_rate = 1;

TEST_F(WrapTest, TestWrapDoesNothingInRange)
{
  auto& wrp = add("core/wrap")
              .set("input", 0.5);
  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  wrp.connect("output", snk, "input");

  run();

  // Should be 1 samples at 0.5
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.5, output[0]);
}

TEST_F(WrapTest, TestWrapWrapsOverRange)
{
  auto& wrp = add("core/wrap")
              .set("max", 0.4)
              .set("input", 0.5);
  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  wrp.connect("output", snk, "input");

  run();

  // Should be 1 samples at 0.1
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_DOUBLE_EQ(0.1, output[0]);
}

TEST_F(WrapTest, TestWrapWrapsUnderRange)
{
  auto& wrp = add("core/wrap")
              .set("min", 0.6)
              .set("input", 0.5);
  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  wrp.connect("output", snk, "input");

  run();

  // Should be 1 samples at 0.9
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_DOUBLE_EQ(0.9, output[0]);
}

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
