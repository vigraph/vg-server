//==========================================================================
// ViGraph dataflow module: maths/limit/test-limit.cc
//
// Tests for limit control
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class LimitTest: public GraphTester
{
public:
  LimitTest()
  {
    loader.load("./vg-module-maths-limit.so");
  }
};

const auto sample_rate = 1;

TEST_F(LimitTest, TestLimitDoesNothingInRange)
{
  auto& lmt = add("maths/limit")
              .set("input", 0.5);
  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  lmt.connect("output", snk, "input");

  run();

  // Should be 1 samples at 0.5
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.5, output[0]);
}

TEST_F(LimitTest, TestLimitCapsOverRange)
{
  auto& lmt = add("maths/limit")
              .set("max", 0.4)
              .set("input", 0.5);
  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  lmt.connect("output", snk, "input");

  run();

  // Should be 1 samples at 0.4
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.4, output[0]);
}

TEST_F(LimitTest, TestLimitCollarsUnderRange)
{
  auto& lmt = add("maths/limit")
              .set("min", 0.6)
              .set("input", 0.5);
  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  lmt.connect("output", snk, "input");

  run();

  // Should be 1 samples at 0.6
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.6, output[0]);
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
