//==========================================================================
// ViGraph dataflow module: core/count/test-count.cc
//
// Tests for count control
//
// Copyright (c) 2018-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
ModuleLoader loader;

const auto sample_rate = 100;

TEST(EnvelopeTest, TestQuiescentProducesZero)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& count = tester.add("count");
  tester.capture_from(count, "output");

  tester.run();

  const auto output = tester.get_output();

  // Should be 100 samples at 0
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ(0.0, output[i]);
}

TEST(EnvelopeTest, TestTriggerUpAndDown)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& count = tester.add("count");

  auto up_data = vector<double>(100);
  up_data[0] = 1.0;
  up_data[33]= 3.0;
  up_data[99]= 1.0;
  auto& ups = tester.add_source(up_data);
  ups.connect("output", count, "up");

  auto down_data = vector<double>(100);
  down_data[5] = 1.0;
  down_data[33]= 1.0;  // Note same sample as up
  auto& dns = tester.add_source(down_data);
  dns.connect("output", count, "down");

  tester.capture_from(count, "output");

  tester.run();

  const auto output = tester.get_output();

  // Should be 100 samples at various steps
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
  {
    if (i<5)
      EXPECT_EQ(1.0, output[i]) << i;
    else if (i<33)
      EXPECT_EQ(0.0, output[i]) << i;
    else if (i<99)
      EXPECT_EQ(2.0, output[i]) << i;
    else
      EXPECT_EQ(3.0, output[i]) << i;
  }
}

TEST(EnvelopeTest, TestDeltaUpDown)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& count = tester.add("count")
                      .set("delta", 3.14);

  auto up_data = vector<double>(100);
  up_data[0] = 1.0;
  auto& ups = tester.add_source(up_data);
  ups.connect("output", count, "up");

  auto down_data = vector<double>(100);
  down_data[50]= 1.0;
  auto& dns = tester.add_source(down_data);
  dns.connect("output", count, "down");

  tester.capture_from(count, "output");

  tester.run();

  const auto output = tester.get_output();

  // Should be 50 samples at 3.14, 50 at 0
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
  {
    if (i<50)
      EXPECT_EQ(3.14, output[i]) << i;
    else
      EXPECT_EQ(0.0, output[i]) << i;
  }
}

TEST(EnvelopeTest, TestReset)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& count = tester.add("count");

  auto up_data = vector<double>(100);
  up_data[0] = 1.0;
  auto& ups = tester.add_source(up_data);
  ups.connect("output", count, "up");

  auto reset_data = vector<double>(100);
  reset_data[50]= 1.0;
  auto& res = tester.add_source(reset_data);
  res.connect("output", count, "reset");

  tester.capture_from(count, "output");

  tester.run();

  const auto output = tester.get_output();

  // Should be 50 samples at 1.0, 50 at 0
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
  {
    if (i<50)
      EXPECT_EQ(1.0, output[i]) << i;
    else
      EXPECT_EQ(0.0, output[i]) << i;
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
  loader.load("./vg-module-core-count.so");
  return RUN_ALL_TESTS();
}
