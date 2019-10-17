//==========================================================================
// ViGraph dataflow module: core/random/test-random.cc
//
// Tests for random control
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
ModuleLoader loader;

const auto sample_rate = 100;

TEST(RandomTest, TestRandomDefaultFreeRuns0to1)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& compare = tester.add("random");
  tester.capture_from(compare, "output");

  tester.run();

  const auto output = tester.get_output();

  // Should be 100 samples between 0..1
  ASSERT_EQ(sample_rate, output.size());
  for(auto i=0u; i<sample_rate; i++)
  {
    EXPECT_LT(0.0, output[i]);  // Test for 0, vanishing unlikely to happen
    EXPECT_GE(1.0, output[i]);
  }
}

TEST(RandomTest, TestRandomWithTriggerDoesntStartUntilTriggerAndHolds)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& compare = tester.add("random");
  auto trigger_data = vector<double>(100);
  trigger_data[50] = 1.0;
  auto& ts = tester.add_source(trigger_data);
  ts.connect("output", compare, "trigger");
  tester.capture_from(compare, "output");

  tester.run();

  const auto output = tester.get_output();

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

TEST(RandomTest, TestRandomSpecifiedRange)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& compare = tester.add("random")
                        .set("min", 100.0)
                        .set("max", 200.0);
  tester.capture_from(compare, "output");

  tester.run();

  const auto output = tester.get_output();

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
  loader.load("./vg-module-core-random.so");
  return RUN_ALL_TESTS();
}



