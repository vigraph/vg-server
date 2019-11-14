//==========================================================================
// ViGraph dataflow module: core/interpolator/test-interpolator.cc
//
// Tests for <interpolator> control
//
// Copyright (c) 2018-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class InterpolatorTest: public GraphTester
{
public:
  InterpolatorTest()
  {
    loader.load("./vg-module-core-interpolator.so");
  }
};

const auto sample_rate = 100;

TEST_F(InterpolatorTest, TestWithNoPeriodGeneratesFrom)
{
  auto& interp = add("interpolator").set("from", 42.0);
  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  interp.connect("output", snk, "input");

  run();

  // Should be 100 samples at 42.0
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ(42.0, output[i]) << i;

}

TEST_F(InterpolatorTest, TestAutoRun1Sec)
{
  auto& interp = add("interpolator").set("to", 1.0).set("period", 1.0);
  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  interp.connect("output", snk, "input");

  run();

  // Should be 100 samples linearly increasing
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_NEAR(i/100.0, output[i], 1e-8) << i;

}

TEST_F(InterpolatorTest, TestAutoRun1SecWithOverrun)
{
  auto& interp = add("interpolator").set("to", 1.0).set("period", 0.5);
  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  interp.connect("output", snk, "input");

  run();

  // Should be 50 samples linearly increasing, then 50 flat
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_NEAR((i>=50)?1.0:i/50.0, output[i], 1e-8) << i;

}

TEST_F(InterpolatorTest, TestStartConnectedButNotTriggeredGeneratesFrom)
{
  auto& interp = add("interpolator")
    .set("from", 42.0)
    .set("to", 100.0)
    .set("period", 1.0);

  auto start_data = vector<double>(100);
  auto& sts = add_source(start_data);
  sts.connect("output", interp, "start");

  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  interp.connect("output", snk, "input");

  run();

  // Should be 100 samples at 42.0
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ(42.0, output[i]) << i;
}

TEST_F(InterpolatorTest, TestStartTriggers)
{
  auto& interp = add("interpolator")
    .set("from", 1.0)
    .set("to", 2.0)
    .set("period", 0.5);

  auto start_data = vector<double>(100);
  start_data[50] = 1;
  auto& sts = add_source(start_data);
  sts.connect("output", interp, "start");

  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  interp.connect("output", snk, "input");

  run();

  // Should be 50 samples at 1.0, then 50 increasing to 2.0
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_NEAR((i<50)?1.0 : 1.0+(i-50)/50.0, output[i], 1e-8) << i;
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
