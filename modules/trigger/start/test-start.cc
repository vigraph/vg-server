//==========================================================================
// ViGraph dataflow module: trigger/start/test-start.cc
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
    loader.load("./vg-module-trigger-start.so");
  }
};

const auto sample_rate = 1;

TEST_F(StartTest, TestTriggeredAtStartAndThenNot)
{
  auto& start = add("trigger/start");

  auto output = vector<Trigger>{};
  auto& sink = add_sink(output, sample_rate);
  start.connect("output", sink, "input");

  run(2);

  ASSERT_EQ(2, output.size());
  EXPECT_EQ(1, output[0]);
  EXPECT_EQ(0, output[1]);
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
