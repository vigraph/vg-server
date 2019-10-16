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

TEST(EnvelopeTest, TestStartHalfWayThrough)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& env = tester.add("envelope");

  auto start_data = vector<double>(100);
  start_data[50] = 1.0;
  auto& sts = tester.add_source(start_data);
  sts.connect("output", env, "start");

  tester.capture_from(env, "output");

  tester.run();

  const auto output = tester.get_output();

  // Should be 50 samples at 0.0, 50 at 1.0
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ((i<sample_rate/2)?0.0:1.0, output[i]) << i;
}

TEST(EnvelopeTest, TestStartThenStopHalfWayThrough)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& env = tester.add("envelope");

  auto start_data = vector<double>(100);
  start_data[0] = 1.0;
  auto& sts = tester.add_source(start_data);
  sts.connect("output", env, "start");

  auto stop_data = vector<double>(100);
  stop_data[50] = 1.0;
  auto& sos = tester.add_source(stop_data);
  sos.connect("output", env, "stop");

  tester.capture_from(env, "output");

  tester.run();

  const auto output = tester.get_output();

  // Should be 50 samples at 1.0, 50 at 0.0
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ((i<sample_rate/2)?1.0:0.0, output[i]) << i;
}

TEST(EnvelopeTest, TestAttack)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& env = tester.add("envelope")
                    .set("attack", 1.0);
  auto start_data = vector<double>(100);
  start_data[0] = 1.0;
  auto& sts = tester.add_source(start_data);
  sts.connect("output", env, "start");

  tester.capture_from(env, "output");

  tester.run();

  const auto output = tester.get_output();

  // Should be 100 samples, linearly increasing
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_NEAR((double)i/sample_rate, output[i], 0.01) << i;
}

TEST(EnvelopeTest, TestDecayToZero)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& env = tester.add("envelope")
                    .set("decay", 1.0)
                    .set("sustain", 0.0);
  auto start_data = vector<double>(100);
  start_data[0] = 1.0;
  auto& sts = tester.add_source(start_data);
  sts.connect("output", env, "start");

  tester.capture_from(env, "output");

  tester.run();

  const auto output = tester.get_output();

  // Should be 100 samples, linearly decreasing
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_NEAR(1.0-(double)i/sample_rate, output[i], 0.01) << i;
}

TEST(EnvelopeTest, TestDecayToSustain)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& env = tester.add("envelope")
                    .set("decay", 0.5)
                    .set("sustain", 0.5);
  auto start_data = vector<double>(100);
  start_data[0] = 1.0;
  auto& sts = tester.add_source(start_data);
  sts.connect("output", env, "start");

  tester.capture_from(env, "output");

  tester.run();

  const auto output = tester.get_output();

  // Should be 100 samples, linearly decreasing to 0.5, then stable
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
  {
    if (i < sample_rate/2)
      EXPECT_NEAR(1.0-(double)i/sample_rate, output[i], 0.01) << i;
    else
      EXPECT_EQ(0.5, output[i]) << i;
  }
}

TEST(EnvelopeTest, TestRelease)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& env = tester.add("envelope")
                    .set("release", 0.5);

  auto start_data = vector<double>(100);
  start_data[0] = 1.0;
  auto& sts = tester.add_source(start_data);
  sts.connect("output", env, "start");

  auto stop_data = vector<double>(100);
  stop_data[50] = 1.0;
  auto& sos = tester.add_source(stop_data);
  sos.connect("output", env, "stop");

  tester.capture_from(env, "output");

  tester.run();

  const auto output = tester.get_output();

  // Should be 100 samples, linearly decreasing
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
  {
    if (i < sample_rate/2)
      EXPECT_EQ(1.0, output[i]) << i;
    else
      EXPECT_NEAR(1.0-2.0*(i-sample_rate/2)/sample_rate, output[i], 0.01) << i;
  }
}

TEST(EnvelopeTest, TestFinishedTriggeredAfterRelease)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& env = tester.add("envelope")
                    .set("release", 0.25);

  auto start_data = vector<double>(100);
  start_data[0] = 1.0;
  auto& sts = tester.add_source(start_data);
  sts.connect("output", env, "start");

  auto stop_data = vector<double>(100);
  stop_data[50] = 1.0;
  auto& sos = tester.add_source(stop_data);
  sos.connect("output", env, "stop");

  tester.capture_from(env, "finished");

  tester.run();

  const auto output = tester.get_output();

  // Should be 100 samples, all 0 except #75 (=stop at 0.5 + 0.25 for release)
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ((i==75)?1.0:0.0, output[i]) << i;
}

TEST(EnvelopeTest, TestReleaseStartsFromCurrentPositionInAttack)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& env = tester.add("envelope")
                    .set("attack", 1.0)
                    .set("release", 0.5);

  auto start_data = vector<double>(100);
  start_data[0] = 1.0;
  auto& sts = tester.add_source(start_data);
  sts.connect("output", env, "start");

  auto stop_data = vector<double>(100);
  stop_data[50] = 1.0;
  auto& sos = tester.add_source(stop_data);
  sos.connect("output", env, "stop");

  tester.capture_from(env, "output");

  tester.run();

  const auto output = tester.get_output();

  // Should be 100 samples, rising to 0.49 at 50, then decreasing to 0 again
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
  {
    if (i < sample_rate/2)
      EXPECT_NEAR((double)i/sample_rate, output[i], 0.01) << i;
    else
      EXPECT_NEAR(0.49-((double)i-sample_rate/2)/sample_rate, output[i], 0.01)
        << i;
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
  loader.load("./vg-module-core-envelope.so");
  return RUN_ALL_TESTS();
}
