//==========================================================================
// ViGraph dataflow module: envelope/test-envelope.cc
//
// Tests for <envelope> control
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
ModuleLoader loader;

const auto sample_rate = 100;

TEST(EnvelopeTest, TestUnsetNotStartedProducesZero)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& env = tester.add("envelope");
  tester.capture_from(env, "output");

  tester.run();

  const auto output = tester.get_output();

  // Should be 100 samples at 0
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ(0.0, output[i]);
}

TEST(EnvelopeTest, TestUnsetStartedProduces1)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& env = tester.add("envelope")
                    .set("start", 1.0);
  tester.capture_from(env, "output");

  tester.run();

  const auto output = tester.get_output();

  // Should be 100 samples at 1.0
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ(1.0, output[i]);
}

TEST(EnvelopeTest, TestSustainSetStartedProducesSustain)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& env = tester.add("envelope")
                    .set("sustain", 0.75)
                    .set("start", 1.0);
  tester.capture_from(env, "output");

  tester.run();

  const auto output = tester.get_output();

  // Should be 100 samples at 0.75
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ(0.75, output[i]);
}

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  loader.load("./vg-module-core-envelope.so");
  return RUN_ALL_TESTS();
}
