//==========================================================================
// ViGraph dataflow module: envelope/test-envelope.cc
//
// Tests for <envelope> control
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class EnvelopeTest: public GraphTester
{
public:
  EnvelopeTest()
  {
    loader.load("./vg-module-audio-envelope.so");
  }
};

const auto nsamples = 100;

TEST_F(EnvelopeTest, TestUnsetNotStartedProducesZero)
{
  const auto expected = vector<double>(nsamples, 0.0);
  auto actual = vector<double>{};

  auto& env = add("audio/envelope");
  auto& snk = add_sink(actual, nsamples);
  env.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected, actual);
}

TEST_F(EnvelopeTest, TestUnsetStartedProduces1)
{
  const auto expected = vector<double>(nsamples, 1.0);
  auto actual = vector<double>{};

  auto& env = add("audio/envelope")
              .set("start", Trigger{1});
  auto& snk = add_sink(actual, nsamples);
  env.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected, actual);
}

TEST_F(EnvelopeTest, TestSustainSetStartedProducesSustain)
{
  const auto expected = vector<double>(nsamples, 0.75);
  auto actual = vector<double>{};

  auto& env = add("audio/envelope")
              .set("sustain", 0.75)
              .set("start", Trigger{1});
  auto& snk = add_sink(actual, nsamples);
  env.connect("output", snk, "input");

  run();

  EXPECT_EQ(expected, actual);
}

TEST_F(EnvelopeTest, TestStartHalfWayThrough)
{
  auto& env = add("audio/envelope");

  auto start_data = vector<Trigger>(100);
  start_data[50] = 1;
  auto& sts = add_source(start_data);
  sts.connect("output", env, "start");
  auto output = vector<double>{};
  auto& snk = add_sink(output, nsamples);
  env.connect("output", snk, "input");

  run();

  // Should be 50 samples at 0.0, 50 at 1.0
  EXPECT_EQ(nsamples, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ((i<nsamples/2)?0.0:1.0, output[i]) << i;
}

TEST_F(EnvelopeTest, TestStartThenStopHalfWayThrough)
{
  auto& env = add("audio/envelope");

  auto start_data = vector<Trigger>(100);
  start_data[0] = 1;
  auto& sts = add_source(start_data);
  sts.connect("output", env, "start");

  auto stop_data = vector<Trigger>(100);
  stop_data[50] = 1;
  auto& sos = add_source(stop_data);
  sos.connect("output", env, "stop");
  auto output = vector<double>{};
  auto& snk = add_sink(output, nsamples);
  env.connect("output", snk, "input");

  run();

  // Should be 50 samples at 1.0, 50 at 0.0
  EXPECT_EQ(nsamples, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ((i<nsamples/2)?1.0:0.0, output[i]) << i;
}

TEST_F(EnvelopeTest, TestAttack)
{
  auto& env = add("audio/envelope")
              .set("attack", 1.0);
  auto start_data = vector<Trigger>(100);
  start_data[0] = 1;
  auto& sts = add_source(start_data);
  sts.connect("output", env, "start");
  auto output = vector<double>{};
  auto& snk = add_sink(output, nsamples);
  env.connect("output", snk, "input");

  run();

  // Should be 100 samples, linearly increasing
  EXPECT_EQ(nsamples, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_NEAR((double)i/nsamples, output[i], 0.01) << i;
}

TEST_F(EnvelopeTest, TestDecayToZero)
{
  auto& env = add("audio/envelope")
              .set("decay", 1.0)
              .set("sustain", 0.0);
  auto start_data = vector<Trigger>(100);
  start_data[0] = 1;
  auto& sts = add_source(start_data);
  sts.connect("output", env, "start");
  auto output = vector<double>{};
  auto& snk = add_sink(output, nsamples);
  env.connect("output", snk, "input");

  run();

  // Should be 100 samples, linearly decreasing
  EXPECT_EQ(nsamples, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_NEAR(1.0-(double)i/nsamples, output[i], 0.01) << i;
}

TEST_F(EnvelopeTest, TestDecayToSustain)
{
  auto& env = add("audio/envelope")
              .set("decay", 0.5)
              .set("sustain", 0.5);
  auto start_data = vector<Trigger>(100);
  start_data[0] = 1;
  auto& sts = add_source(start_data);
  sts.connect("output", env, "start");
  auto output = vector<double>{};
  auto& snk = add_sink(output, nsamples);
  env.connect("output", snk, "input");

  run();

  // Should be 100 samples, linearly decreasing to 0.5, then stable
  EXPECT_EQ(nsamples, output.size());
  for(auto i=0u; i<output.size(); i++)
  {
    if (i < nsamples/2)
      EXPECT_NEAR(1.0-(double)i/nsamples, output[i], 0.01) << i;
    else
      EXPECT_EQ(0.5, output[i]) << i;
  }
}

TEST_F(EnvelopeTest, TestRelease)
{
  auto& env = add("audio/envelope")
              .set("release", 0.5);

  auto start_data = vector<Trigger>(100);
  start_data[0] = 1;
  auto& sts = add_source(start_data);
  sts.connect("output", env, "start");

  auto stop_data = vector<Trigger>(100);
  stop_data[50] = 1;
  auto& sos = add_source(stop_data);
  sos.connect("output", env, "stop");
  auto output = vector<double>{};
  auto& snk = add_sink(output, nsamples);
  env.connect("output", snk, "input");

  run();

  // Should be 100 samples, linearly decreasing
  EXPECT_EQ(nsamples, output.size());
  for(auto i=0u; i<output.size(); i++)
  {
    if (i < nsamples/2)
      EXPECT_EQ(1.0, output[i]) << i;
    else
      EXPECT_NEAR(1.0-2.0*(i-nsamples/2)/nsamples, output[i], 0.01) << i;
  }
}

TEST_F(EnvelopeTest, TestFinishedTriggeredAfterRelease)
{
  auto& env = add("audio/envelope")
              .set("release", 0.25);

  auto start_data = vector<Trigger>(100);
  start_data[0] = 1;
  auto& sts = add_source(start_data);
  sts.connect("output", env, "start");

  auto stop_data = vector<Trigger>(100);
  stop_data[50] = 1;
  auto& sos = add_source(stop_data);
  sos.connect("output", env, "stop");

  auto output = vector<Trigger>{};
  auto& snk = add_sink(output, nsamples);
  env.connect("finished", snk, "input");

  run();

  // Should be 100 samples, all 0 except #75 (=stop at 0.5 + 0.25 for release)
  EXPECT_EQ(nsamples, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ((i==75)?1.0:0.0, output[i]) << i;
}

TEST_F(EnvelopeTest, TestReleaseStartsFromCurrentPositionInAttack)
{
  auto& env = add("audio/envelope")
              .set("attack", 1.0)
              .set("release", 0.5);

  auto start_data = vector<Trigger>(100);
  start_data[0] = 1;
  auto& sts = add_source(start_data);
  sts.connect("output", env, "start");

  auto stop_data = vector<Trigger>(100);
  stop_data[50] = 1;
  auto& sos = add_source(stop_data);
  sos.connect("output", env, "stop");
  auto output = vector<double>{};
  auto& snk = add_sink(output, nsamples);
  env.connect("output", snk, "input");

  run();

  // Should be 100 samples, rising to 0.49 at 50, then decreasing to 0 again
  EXPECT_EQ(nsamples, output.size());
  for(auto i=0u; i<output.size(); i++)
  {
    if (i < nsamples/2)
      EXPECT_NEAR((double)i/nsamples, output[i], 0.01) << i;
    else
      EXPECT_NEAR(0.49-((double)i-nsamples/2)/nsamples, output[i], 0.01)
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
  return RUN_ALL_TESTS();
}
