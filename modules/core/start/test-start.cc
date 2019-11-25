//==========================================================================
// ViGraph dataflow module: core/start/test-start.cc
//
// Tests for start control
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class StartTest: public GraphTester
{
public:
  StartTest()
  {
    loader.load("./vg-module-core-start.so");
  }
};

const auto sample_rate = 1;

TEST_F(StartTest, TestTriggeredAtStart)
{
  auto& start = add("core/start");

  auto output = vector<double>{};
  auto& sink = add_sink(output, sample_rate);
  start.connect("output", sink, "input");

  run(1);

  ASSERT_EQ(1, output.size());
  EXPECT_EQ(1, output[0]);
}

TEST_F(StartTest, TestNotTriggeredAfterStart)
{
  auto& start = add("core/start");

  auto output = vector<double>{};
  auto& sink = add_sink(output, sample_rate);
  start.connect("output", sink, "input");

  run(2);

  ASSERT_EQ(1, output.size());
  EXPECT_EQ(0, output[0]);
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
