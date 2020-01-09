//==========================================================================
// ViGraph dataflow module: trigger/beat/test-beat.cc
//
// Tests for <beat> control
//
// Copyright (c) 2018-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class BeatTest: public GraphTester
{
public:
  BeatTest()
  {
    loader.load("./vg-module-trigger-beat.so");
  }
};

const auto sample_rate = 100;

TEST_F(BeatTest, TestInterval)
{
  auto& bet = add("trigger/beat")
              .set("interval", 0.1);
  auto output = vector<Trigger>{};
  auto& snk = add_sink(output, sample_rate);
  bet.connect("output", snk, "input");

  run();

  // Should be 100 samples at 0, with 1's every 10
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ((i%10)?0.0:1.0, output[i]) << i;

}

TEST_F(BeatTest, TestIntervalWithOffset)
{
  auto& bet = add("trigger/beat")
              .set("interval", 0.1)
              .set("offset",   0.05);
  auto output = vector<Trigger>{};
  auto& snk = add_sink(output, sample_rate);
  bet.connect("output", snk, "input");

  run();

  // Should be 100 samples at 0, with 1's every 10, starting at 5
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ(((i+5)%10)?0.0:1.0, output[i]) << i;

}

TEST_F(BeatTest, TestStartingMidwayThroughTick)
{
  auto& bet = add("trigger/beat")
              .set("interval", 0.1);

  auto start_data = vector<Trigger>(100);
  start_data[50] = 1;
  auto& sts = add_source(start_data);
  sts.connect("output", bet, "start");
  auto output = vector<Trigger>{};
  auto& snk = add_sink(output, sample_rate);
  bet.connect("output", snk, "input");

  run();

  // Should be 100 samples at 0, with 1's every 10, only after 50
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ((i<50 || i%10)?0.0:1.0, output[i]) << i;
}

TEST_F(BeatTest, TestStoppingMidwayThroughTick)
{
  auto& bet = add("trigger/beat")
              .set("interval", 0.1);

  auto start_data = vector<Trigger>(100);
  start_data[0] = 1;
  auto& sts = add_source(start_data);
  sts.connect("output", bet, "start");

  auto stop_data = vector<Trigger>(100);
  stop_data[50] = 1;
  auto& sos = add_source(stop_data);
  sos.connect("output", bet, "stop");

  auto output = vector<Trigger>{};
  auto& snk = add_sink(output, sample_rate);
  bet.connect("output", snk, "input");

  run();

  // Should be 100 samples at 0, with 1's every 10, only until 50
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ((i>=50 || i%10)?0.0:1.0, output[i]) << i;
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
