//==========================================================================
// ViGraph dataflow module: time-series/web-fetch/test-web-fetch.cc
//
// Tests for web-fetch filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../time-series-module.h"
#include "../../module-test.h"

class WebFetchTest: public GraphTester
{
public:
  WebFetchTest()
  {
    loader.load("./vg-module-time-series-web-fetch.so");
  }
};

const auto sample_rate = 1;

TEST_F(WebFetchTest, TestFetchWoodForTreesWTIData)
{
  auto& web = add("time-series/web-fetch")
              .set("url", string("http://woodfortrees.org/data/wti"))
              .set("name", string("WTI"));

  auto data = vector<DataSet>{};
  auto& snk = add_sink(data, sample_rate);
  web.connect("output", snk, "input");

  run();

  ASSERT_EQ(1, data.size());
  auto& data0 = data[0];
  ASSERT_LE(487, data0.samples.size());  // Was 487 in Oct'19, will grow

  // Check values for sanity
  for(auto i=0u; i<data0.samples.size(); i++)
  {
    EXPECT_NEAR(1979+i/12.0, data0.samples[i].at, 1e-2);
    EXPECT_LE(-0.5, data0.samples[i].value) << i << endl;

    // One can only hope this never fails!
    EXPECT_GE(1.5, data0.samples[i].value) << i << endl;
  }

  EXPECT_EQ("WTI", data0.name);
  EXPECT_EQ("http://woodfortrees.org/data/wti", data0.source);
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
