//==========================================================================
// ViGraph dataflow module: core/timer/test-timer.cc
//
// Tests for <timer> control
//
// Copyright (c) 2018-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class TimerTest: public GraphTester
{
public:
  TimerTest()
  {
    loader.load("./vg-module-core-timer.so");
  }
};

const auto sample_rate = 100;

TEST_F(TimerTest, TestFreeRun)
{
  auto& timer = add("core/timer");
  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  timer.connect("output", snk, "input");

  run();

  // Should be 100 samples at 0..1
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_NEAR(i/100.0, output[i], 1e-8) << i;

}

TEST_F(TimerTest, TestStartConnectedButNotFired)
{
  auto& timer = add("core/timer");

  auto start_data = vector<double>(100);
  auto& sts = add_source(start_data);
  sts.connect("output", timer, "start");
  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  timer.connect("output", snk, "input");

  run();

  // Should be 100 samples at 0
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ(0, output[i]) << i;
}

TEST_F(TimerTest, TestStartAndRestart)
{
  auto& timer = add("core/timer");

  auto start_data = vector<double>(100);
  start_data[0] = 1.0;
  start_data[50] = 1.0;
  auto& sts = add_source(start_data);
  sts.connect("output", timer, "start");

  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  timer.connect("output", snk, "input");

  run();

  // Should be 50 samples at 0..0.5 then another 50 the same
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_NEAR(i%50/100.0, output[i], 1e-8) << i;
}

TEST_F(TimerTest, TestStartAndStop)
{
  auto& timer = add("core/timer");

  auto start_data = vector<double>(100);
  start_data[25] = 1.0;
  auto& sts = add_source(start_data);
  sts.connect("output", timer, "start");

  auto stop_data = vector<double>(100);
  stop_data[75] = 1.0;
  auto& sos = add_source(stop_data);
  sos.connect("output", timer, "stop");

  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  timer.connect("output", snk, "input");

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
