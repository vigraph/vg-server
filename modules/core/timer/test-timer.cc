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

TEST_F(TimerTest, TestUnconnectedDoesNotStart)
{
  auto& timer = add("core/timer").set("period", 0.5);

  auto finished = vector<Trigger>{};
  auto& sinkf = add_sink(finished, sample_rate);
  timer.connect("finished", sinkf, "input");

  auto active = vector<Number>{};
  auto& sinka = add_sink(active, sample_rate);
  timer.connect("active", sinka, "input");

  run();

  // Should be 100 samples, active until 50, finished at 50
  EXPECT_EQ(sample_rate, finished.size());
  EXPECT_EQ(sample_rate, active.size());
  for(auto i=0u; i<sample_rate; i++)
  {
    EXPECT_EQ(0, finished[i]) << i;
    EXPECT_EQ(0, active[i]) << i;
  }
}

TEST_F(TimerTest, TestRunAfterStart)
{
  auto& timer = add("core/timer").set("period", 0.5);

  auto start_data = vector<Trigger>(100);
  start_data[0] = 1.0;
  auto& sts = add_source(start_data);
  sts.connect("output", timer, "start");

  auto finished = vector<Trigger>{};
  auto& sinkf = add_sink(finished, sample_rate);
  timer.connect("finished", sinkf, "input");

  auto active = vector<Number>{};
  auto& sinka = add_sink(active, sample_rate);
  timer.connect("active", sinka, "input");

  run();

  // Should be 100 samples, active until 50, finished at 50
  EXPECT_EQ(sample_rate, finished.size());
  EXPECT_EQ(sample_rate, active.size());
  for(auto i=0u; i<sample_rate; i++)
  {
    EXPECT_EQ((i==50)?1:0, finished[i]) << i;
    EXPECT_EQ((i<50)?1:0,  active[i]) << i;
  }
}

TEST_F(TimerTest, TestStartThenReset)
{
  auto& timer = add("core/timer").set("period", 0.5);

  auto start_data = vector<Trigger>(100);
  start_data[0] = 1.0;
  auto& sts = add_source(start_data);
  sts.connect("output", timer, "start");

  auto reset_data = vector<Trigger>(100);
  reset_data[25] = 1.0;
  auto& sos = add_source(reset_data);
  sos.connect("output", timer, "reset");

  auto finished = vector<Trigger>{};
  auto& sinkf = add_sink(finished, sample_rate);
  timer.connect("finished", sinkf, "input");

  auto active = vector<Number>{};
  auto& sinka = add_sink(active, sample_rate);
  timer.connect("active", sinka, "input");

  run();

  // Should be 100 samples, active until 25, never finished
  EXPECT_EQ(sample_rate, finished.size());
  EXPECT_EQ(sample_rate, active.size());
  for(auto i=0u; i<sample_rate; i++)
  {
    EXPECT_EQ(0, finished[i]) << i;
    EXPECT_EQ((i<25)?1:0,  active[i]) << i;
  }
}

TEST_F(TimerTest, TestRunAfterRestart)
{
  auto& timer = add("core/timer").set("period", 0.5);

  auto start_data = vector<Trigger>(100);
  start_data[0] = 1.0;
  start_data[25] = 1.0;
  auto& sts = add_source(start_data);
  sts.connect("output", timer, "start");

  auto finished = vector<Trigger>{};
  auto& sinkf = add_sink(finished, sample_rate);
  timer.connect("finished", sinkf, "input");

  auto active = vector<Number>{};
  auto& sinka = add_sink(active, sample_rate);
  timer.connect("active", sinka, "input");

  run();

  // Should be 100 samples, active until 75, finished at 75
  EXPECT_EQ(sample_rate, finished.size());
  EXPECT_EQ(sample_rate, active.size());
  for(auto i=0u; i<sample_rate; i++)
  {
    EXPECT_EQ((i==75)?1:0, finished[i]) << i;
    EXPECT_EQ((i<75)?1:0,  active[i]) << i;
  }
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
