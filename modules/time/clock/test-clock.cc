//==========================================================================
// ViGraph dataflow module: time/clock/test-clock.cc
//
// Tests for <clock> control
//
// Copyright (c) 2018-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class ClockTest: public GraphTester
{
public:
  ClockTest()
  {
    loader.load("./vg-module-time-clock.so");
  }
};

const auto sample_rate = 100;

TEST_F(ClockTest, TestFreeRun)
{
  auto& clock = add("time/clock");
  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  clock.connect("output", snk, "input");

  run();

  // Should be 100 samples at 0..1
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_NEAR(i/100.0, output[i], 1e-8) << i;

}

TEST_F(ClockTest, TestStartConnectedButNotFired)
{
  auto& clock = add("time/clock");

  auto start_data = vector<Trigger>(100);
  auto& sts = add_source(start_data);
  sts.connect("output", clock, "start");
  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  clock.connect("output", snk, "input");

  run();

  // Should be 100 samples at 0
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ(0, output[i]) << i;
}

TEST_F(ClockTest, TestStartAndRestart)
{
  auto& clock = add("time/clock");

  auto start_data = vector<Trigger>(100);
  start_data[0] = 1;
  start_data[50] = 1;
  auto& sts = add_source(start_data);
  sts.connect("output", clock, "start");

  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  clock.connect("output", snk, "input");

  run();

  // Should be 50 samples at 0..0.5 then another 50 the same
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_NEAR(i%50/100.0, output[i], 1e-8) << i;
}

TEST_F(ClockTest, TestStartAndStop)
{
  auto& clock = add("time/clock");

  auto start_data = vector<Trigger>(100);
  start_data[25] = 1;
  auto& sts = add_source(start_data);
  sts.connect("output", clock, "start");

  auto stop_data = vector<Trigger>(100);
  stop_data[75] = 1;
  auto& sos = add_source(stop_data);
  sos.connect("output", clock, "stop");

  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  clock.connect("output", snk, "input");

  run();

  // Should be 25 samples at 0, 50 at 0..0.5, 25 at 0
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_NEAR((i<25 || i>=75)?0:((i-25)/100.0), output[i], 1e-8) << i;
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
