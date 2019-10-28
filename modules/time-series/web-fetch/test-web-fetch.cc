//==========================================================================
// ViGraph dataflow module: time-series/web-fetch/test-web-fetch.cc
//
// Tests for web-fetch filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module.h"
#include "../../module-test.h"

ModuleLoader loader;

const auto sample_rate = 100;

TEST(WebFetchTest, TestFetchWoodForTreesWTIData)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& web = tester.add("time-series/web-fetch")
                    .set("url", string("http://woodfortrees.org/data/wti"));

  tester.capture_from(web, "output");

  tester.run();

  const auto data = tester.get_output();
  ASSERT_LE(487, data.size());  // Was 487 in Oct'19, will grow

  // Check values for sanity
  for(auto i=0u; i<data.size(); i++)
  {
    EXPECT_LE(-0.5, data[i]) << i << " " << data[i] << endl;

    // One can only hope this never fails!
    EXPECT_GE(1.5, data[i]) << i << " " << data[i] << endl;
  }
}

TEST(WebFetchTest, TestFetchWoodForTreesWTIFrom)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& web = tester.add("time-series/web-fetch")
                    .set("url", string("http://woodfortrees.org/data/wti"));

  tester.capture_from(web, "from");

  tester.run();

  const auto data = tester.get_output();
  ASSERT_EQ(1, data.size());
  EXPECT_EQ(1979.0, data[0]);  // Should never change unless WTI is pruned
}

TEST(WebFetchTest, TestFetchWoodForTreesWTIInterval)
{
  GraphTester<double> tester{loader, sample_rate};

  auto& web = tester.add("time-series/web-fetch")
                    .set("url", string("http://woodfortrees.org/data/wti"));

  tester.capture_from(web, "interval");

  tester.run();

  const auto data = tester.get_output();
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
  loader.load("./vg-module-time-series-web-fetch.so");
  return RUN_ALL_TESTS();
}
