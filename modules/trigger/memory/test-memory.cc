//==========================================================================
// ViGraph dataflow module: trigger/memory/test-memory.cc
//
// Tests for memory control
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class MemoryTest: public GraphTester
{
public:
  MemoryTest()
  {
    loader.load("./vg-module-trigger-memory.so");
  }
};

const auto sample_rate = 1;

TEST_F(MemoryTest, TestTriggerIsPassedThroughOnSecondTickOnly)
{
  auto& memory = add("trigger/memory");

  auto input_data = vector<Trigger>(3);  // Three ticks
  input_data[0] = 1.0;
  auto& is = add_source(input_data);
  is.connect("output", memory, "input");

  auto output = vector<Trigger>{};
  auto& sink = add_sink(output, sample_rate);
  memory.connect("output", sink, "input");

  run(3);

  ASSERT_EQ(3, output.size());
  EXPECT_EQ(0, output[0]);
  EXPECT_EQ(1, output[1]);
  EXPECT_EQ(0, output[2]);  // Doesn't stay triggered
}

TEST_F(MemoryTest, TestLoopingTrigger)
{
  auto& memory = add("trigger/memory");

  auto input_data = vector<Trigger>(3);  // Three ticks
  input_data[0] = 1.0;
  auto& is = add_source(input_data);
  is.connect("output", memory, "input");

  auto output = vector<Trigger>{};
  auto& sink = add_sink(output, sample_rate);
  memory.connect("output", sink, "input");

  // Loopback to itself
  memory.connect("output", memory, "input");

  run(3);

  ASSERT_EQ(3, output.size());
  EXPECT_EQ(0, output[0]);
  EXPECT_EQ(1, output[1]);
  EXPECT_EQ(1, output[2]);  // Stays triggered because of loop
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
