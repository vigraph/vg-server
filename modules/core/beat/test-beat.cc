//==========================================================================
// ViGraph dataflow module: core/beat/test-beat.cc
//
// Tests for <beat> control
//
// Copyright (c) 2018-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
ModuleLoader loader;

const auto sample_rate = 100;

TEST(BeatTest, TestInterval)
{
  GraphTester<double> tester(loader, sample_rate);

  auto& beat = tester.add("beat")
                     .set("interval", 0.1);
  tester.capture_from(beat, "output");

  tester.run();

  const auto output = tester.get_output();

  // Should be 100 samples at 0, with 1's every 10
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ((i%10)?0.0:1.0, output[i]) << i;

}

TEST(BeatTest, TestIntervalWithOffset)
{
  GraphTester<double> tester(loader, sample_rate);

  auto& beat = tester.add("beat")
                     .set("interval", 0.1)
                     .set("offset",   0.05);
  tester.capture_from(beat, "output");

  tester.run();

  const auto output = tester.get_output();

  // Should be 100 samples at 0, with 1's every 10, starting at 5
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ(((i+5)%10)?0.0:1.0, output[i]) << i;

}

TEST(BeatTest, TestStartingMidwayThroughTick)
{
  GraphTester<double> tester(loader, sample_rate);

  auto& beat = tester.add("beat")
                     .set("interval", 0.1);

  auto start_data = vector<double>(100);
  start_data[50] = 1.0;
  auto& sts = tester.add_source(start_data);
  sts.connect("output", beat, "start");

  tester.capture_from(beat, "output");

  tester.run();

  const auto output = tester.get_output();

  // Should be 100 samples at 0, with 1's every 10, only after 50
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ((i<50 || i%10)?0.0:1.0, output[i]) << i;
}

TEST(BeatTest, TestStoppingMidwayThroughTick)
{
  GraphTester<double> tester(loader, sample_rate);

  auto& beat = tester.add("beat")
                     .set("interval", 0.1);

  auto start_data = vector<double>(100);
  start_data[0] = 1.0;
  auto& sts = tester.add_source(start_data);
  sts.connect("output", beat, "start");

  auto stop_data = vector<double>(100);
  stop_data[50] = 1.0;
  auto& sos = tester.add_source(stop_data);
  sos.connect("output", beat, "stop");

  tester.capture_from(beat, "output");

  tester.run();

  const auto output = tester.get_output();

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
  loader.load("./vg-module-core-beat.so");
  return RUN_ALL_TESTS();
}
