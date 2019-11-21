//==========================================================================
// ViGraph dataflow module: core/random/test-random.cc
//
// Tests for random control
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class RandomTest: public GraphTester
{
public:
  RandomTest()
  {
    loader.load("./vg-module-core-random.so");
  }
};

const auto sample_rate = 100;

TEST_F(RandomTest, TestRandomDefaultFreeRuns0to1)
{
  auto& rnd = add("core/random");
  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  rnd.connect("output", snk, "input");

  run();

  // Should be 100 samples between 0..1
  ASSERT_EQ(sample_rate, output.size());
  for(auto i=0u; i<sample_rate; i++)
  {
    EXPECT_LT(0.0, output[i]);  // Test for 0, vanishing unlikely to happen
    EXPECT_GE(1.0, output[i]);
  }
}

TEST_F(RandomTest, TestRandomWithTriggerDoesntStartUntilTriggerAndHolds)
{
  auto& rnd = add("core/random");
  auto trigger_data = vector<double>(100);
  trigger_data[50] = 1.0;
  auto& ts = add_source(trigger_data);
  ts.connect("output", rnd, "trigger");
  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  rnd.connect("output", snk, "input");

  run();

  // Should be 100 samples between 0..1
  ASSERT_EQ(sample_rate, output.size());
  for(auto i=0u; i<sample_rate; i++)
  {
    if (i<50)
    {
      EXPECT_EQ(0.0, output[i]);
    }
    else if (i==50)
    {
      EXPECT_LT(0.0, output[i]);
      EXPECT_GE(1.0, output[i]);
    }
    else
    {
      EXPECT_EQ(output[50], output[i]);
    }
  }
}

TEST_F(RandomTest, TestRandomSpecifiedRange)
{
  auto& rnd = add("core/random")
              .set("min", 100.0)
              .set("max", 200.0);
  auto output = vector<double>{};
  auto& snk = add_sink(output, sample_rate);
  rnd.connect("output", snk, "input");

  run();

  // Should be 100 samples between 100 and 200
  ASSERT_EQ(sample_rate, output.size());
  for(auto i=0u; i<sample_rate; i++)
  {
    EXPECT_LE(100.0, output[i]); 
    EXPECT_GE(200.0, output[i]);
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



