//==========================================================================
// ViGraph dataflow module: core/limit/test-limit.cc
//
// Tests for limit control
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
ModuleLoader loader;

const auto sample_rate = 1;

TEST(LimitTest, TestLimitDoesNothingInRange)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& compare = tester.add("limit")
                        .set("input", 0.5);
  tester.capture_from(compare, "output");

  tester.run();

  const auto output = tester.get_output();

  // Should be 1 samples at 0.5
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.5, output[0]);
}

TEST(LimitTest, TestLimitCapsOverRange)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& compare = tester.add("limit")
                        .set("max", 0.4)
                        .set("input", 0.5);
  tester.capture_from(compare, "output");

  tester.run();

  const auto output = tester.get_output();

  // Should be 1 samples at 0.4
  EXPECT_EQ(sample_rate, output.size());
  EXPECT_EQ(0.4, output[0]);
}

TEST(LimitTest, TestLimitCollarsUnderRange)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& compare = tester.add("limit")
                        .set("min", 0.6)
                        .set("input", 0.5);
  tester.capture_from(compare, "output");

  tester.run();

  const auto output = tester.get_output();

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
  loader.load("./vg-module-core-limit.so");
  return RUN_ALL_TESTS();
}
