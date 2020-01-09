//==========================================================================
// ViGraph dataflow module: trigger/count/test-count.cc
//
// Tests for count control
//
// Copyright (c) 2018-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class CountTest: public GraphTester
{
public:
  CountTest()
  {
    loader.load("./vg-module-trigger-count.so");
  }
};

const auto sample_rate = 100;

TEST_F(CountTest, TestQuiescentProducesZero)
{
  auto& cnt = add("trigger/count");
  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  cnt.connect("output", snk, "input");

  run();

  // Should be 100 samples at 0
  EXPECT_EQ(sample_rate, output.size());
  for(auto i=0u; i<output.size(); i++)
    EXPECT_EQ(0.0, output[i]);
}

TEST_F(CountTest, TestTriggerUpAndDown)
{
  auto& cnt = add("trigger/count");

  auto up_data = vector<Trigger>(100);
  up_data[0] = 1;
  up_data[33]= 3;
  up_data[99]= 1;
  auto& ups = add_source(up_data);
  ups.connect("output", cnt, "up");

  auto down_data = vector<Trigger>(100);
  down_data[5] = 1;
  down_data[33]= 1;  // Note same sample as up
  auto& dns = add_source(down_data);
  dns.connect("output", cnt, "down");

  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  cnt.connect("output", snk, "input");

  run();

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

TEST_F(CountTest, TestDeltaUpDown)
{
  auto& cnt = add("trigger/count")
              .set("delta", 3.14);

  auto up_data = vector<Trigger>(100);
  up_data[0] = 1;
  auto& ups = add_source(up_data);
  ups.connect("output", cnt, "up");

  auto down_data = vector<Trigger>(100);
  down_data[50]= 1;
  auto& dns = add_source(down_data);
  dns.connect("output", cnt, "down");

  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  cnt.connect("output", snk, "input");

  run();

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

TEST_F(CountTest, TestReset)
{
  auto& cnt = add("trigger/count");

  auto up_data = vector<Trigger>(100);
  up_data[0] = 1;
  auto& ups = add_source(up_data);
  ups.connect("output", cnt, "up");

  auto reset_data = vector<Trigger>(100);
  reset_data[50]= 1;
  auto& res = add_source(reset_data);
  res.connect("output", cnt, "reset");

  auto output = vector<Number>{};
  auto& snk = add_sink(output, sample_rate);
  cnt.connect("output", snk, "input");

  run();

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
  return RUN_ALL_TESTS();
}
