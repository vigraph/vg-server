//==========================================================================
// ViGraph dataflow module: time/now/test-now.cc
//
// Tests for <now> module
//
// Copyright (c) 2021 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class NowTest: public GraphTester
{
public:
  NowTest()
  {
    loader.load("./vg-module-time-now.so");
  }
};

const auto sample_rate = 1;

TEST_F(NowTest, TestMatchesWallTime)
{
  auto& now = add("time/now");

  auto year_output = vector<Number>{};
  auto& year_sink = add_sink(year_output, sample_rate);
  now.connect("year", year_sink, "input");

  auto month_output = vector<Number>{};
  auto& month_sink = add_sink(month_output, sample_rate);
  now.connect("month", month_sink, "input");

  auto day_output = vector<Number>{};
  auto& day_sink = add_sink(day_output, sample_rate);
  now.connect("day", day_sink, "input");

  auto hour_output = vector<Number>{};
  auto& hour_sink = add_sink(hour_output, sample_rate);
  now.connect("hour", hour_sink, "input");

  auto minute_output = vector<Number>{};
  auto& minute_sink = add_sink(minute_output, sample_rate);
  now.connect("minute", minute_sink, "input");

  auto second_output = vector<Number>{};
  auto& second_sink = add_sink(second_output, sample_rate);
  now.connect("second", second_sink, "input");

  auto time_output = vector<Number>{};
  auto& time_sink = add_sink(time_output, sample_rate);
  now.connect("time", time_sink, "input");

  Time::Stamp tnow = Time::Stamp::now().localise();

  run();

  // All should have 1 sample
  EXPECT_EQ(sample_rate, year_output.size());
  EXPECT_EQ(sample_rate, month_output.size());
  EXPECT_EQ(sample_rate, day_output.size());
  EXPECT_EQ(sample_rate, hour_output.size());
  EXPECT_EQ(sample_rate, minute_output.size());
  EXPECT_EQ(sample_rate, second_output.size());
  EXPECT_EQ(sample_rate, time_output.size());

  Time::Split split;
  split.year = (int)year_output[0];
  split.month = (int)month_output[0];
  split.day = (int)day_output[0];
  split.hour = (int)hour_output[0];
  split.min = (int)minute_output[0];
  split.sec = second_output[0];

  Time::Stamp result(split);

  EXPECT_NEAR(tnow.time(), result.time(), 1);
  EXPECT_EQ(split.hour*100 + split.min, (int)time_output[0]);
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
