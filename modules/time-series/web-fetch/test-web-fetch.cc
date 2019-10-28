//==========================================================================
// ViGraph dataflow module: time-series/web-fetch/test-web-fetch.cc
//
// Tests for web-fetch filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"
#include "../../module-test.h"

class WebFetchTest: public GraphTester
{
public:
  WebFetchTest()
  {
    loader.load("./vg-module-time-series-web-fetch.so");
  }
};

const auto sample_rate = 100;

TEST_F(WebFetchTest, TestFetchWoodForTreesWTIData)
{
  auto& web = add("time-series/web-fetch")
              .set("url", string("http://woodfortrees.org/data/wti"));

  auto data = vector<double>{};
  auto& snk = add_sink(data, sample_rate);
  web.connect("output", snk, "input");

  run();

  ASSERT_LE(487, data.size());  // Was 487 in Oct'19, will grow

  // Check values for sanity
  for(auto i=0u; i<data.size(); i++)
  {
    EXPECT_LE(-0.5, data[i]) << i << " " << data[i] << endl;

    // One can only hope this never fails!
    EXPECT_GE(1.5, data[i]) << i << " " << data[i] << endl;
  }
}

TEST_F(WebFetchTest, TestFetchWoodForTreesWTIFrom)
{
  auto& web = add("time-series/web-fetch")
              .set("url", string("http://woodfortrees.org/data/wti"));

  auto data = vector<double>{};
  auto& snk = add_sink(data, sample_rate);
  web.connect("from", snk, "input");

  run();

  ASSERT_EQ(1, data.size());
  EXPECT_EQ(1979.0, data[0]);  // Should never change unless WTI is pruned
}

TEST_F(WebFetchTest, TestFetchWoodForTreesWTIInterval)
{
  auto& web = add("time-series/web-fetch")
              .set("url", string("http://woodfortrees.org/data/wti"));

  auto data = vector<double>{};
  auto& snk = add_sink(data, sample_rate);
  web.connect("interval", snk, "input");

  run();

  ASSERT_EQ(1, data.size());
  EXPECT_DOUBLE_EQ(1/12.0, data[0]);  // Monthly in decimal years
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
