//==========================================================================
// ViGraph dataflow module: core/cut/test-cut.cc
//
// Tests for cut control
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class CutTest: public GraphTester
{
public:
  CutTest()
  {
    loader.load("./vg-module-core-cut.so");
  }
};

const auto sample_rate = 1;

TEST_F(CutTest, TestTriggerIsNotPassedThroughImmediately)
{
  auto& cut = add("core/cut");

  auto input_data = vector<Trigger>(1);
  input_data[0] = 1.0;
  auto& is = add_source(input_data);
  is.connect("output", cut, "input");

  auto output = vector<Trigger>{};
  auto& sink = add_sink(output, sample_rate);
  cut.connect("output", sink, "input");

  run(1);

  ASSERT_EQ(1, output.size());
  EXPECT_EQ(0, output[0]);
}

TEST_F(CutTest, TestTriggerIsPassedThroughOnSecondTick)
{
  auto& cut = add("core/cut");

  auto input_data = vector<Trigger>(2);  // Two ticks
  input_data[0] = 1.0;
  auto& is = add_source(input_data);
  is.connect("output", cut, "input");

  auto output = vector<Trigger>{};
  auto& sink = add_sink(output, sample_rate);
  cut.connect("output", sink, "input");

  run(2);

  ASSERT_EQ(1, output.size());
  EXPECT_EQ(1, output[0]);
}

TEST_F(CutTest, TestLoopingTrigger)
{
  auto& cut = add("core/cut");

  auto input_data = vector<Trigger>(2);  // Two ticks
  input_data[0] = 1.0;
  auto& is = add_source(input_data);
  is.connect("output", cut, "input");

  auto output = vector<Trigger>{};
  auto& sink = add_sink(output, sample_rate);
  cut.connect("output", sink, "input");

  // Loopback to itself
  cut.connect("output", cut, "input");

  run(2);

  ASSERT_EQ(1, output.size());
  EXPECT_EQ(1, output[0]);
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
