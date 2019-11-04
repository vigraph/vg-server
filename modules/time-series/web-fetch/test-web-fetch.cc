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

  auto datacs = vector<DataCollection>{};
  auto& sink = add_sink(datacs, sample_rate);
  web.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, datacs.size());
  const auto& datac = datacs[0];
  ASSERT_EQ(1, datac.datasets.size());
  auto& data = datac.datasets[0];
  ASSERT_LE(487, data.samples.size());  // Was 487 in Oct'19, will grow

  // Check values for sanity
  for(auto i=0u; i<data.samples.size(); i++)
  {
    EXPECT_NEAR(1979+i/12.0, data.samples[i].at, 1e-2);
    EXPECT_LE(-0.5, data.samples[i].value) << i << endl;

    // One can only hope this never fails!
    EXPECT_GE(1.5, data.samples[i].value) << i << endl;
  }

  EXPECT_EQ("WTI", data.name);
  EXPECT_EQ("http://woodfortrees.org/data/wti", data.source);
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
