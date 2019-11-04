//==========================================================================
// ViGraph dataflow module: time-series/last/test-last.cc
//
// Tests for last time-series filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../time-series-module.h"
#include "../../module-test.h"
#include <cmath>

class LastTest: public GraphTester
{
public:
  LastTest()
  {
    loader.load("./vg-module-time-series-last.so");
  }
};

const auto sample_rate = 1;

TEST_F(LastTest, TestNoChangeIfLessThanNSamples)
{
  auto& last = add("time-series/last")
              .set("samples", 100.0);

  auto datacs = vector<DataCollection>(1);
  auto& datac = datacs[0];
  DataSet data;
  for(auto i=0u; i<50; i++)
    data.add(2000+i, i/50.0);
  datac.add(data);

  auto& ds = add_source(datacs);
  ds.connect("output", last, "input");

  auto rdatacs = vector<DataCollection>{};
  auto& sink = add_sink(rdatacs, sample_rate);
  last.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, rdatacs.size());
  const auto& rdatac = rdatacs[0];
  ASSERT_EQ(1, rdatac.datasets.size());
  const auto& rdata = rdatac.datasets[0];
  ASSERT_EQ(50, rdata.samples.size());
  for(auto i=0u; i<rdata.samples.size(); i++)
  {
    const auto& s = rdata.samples[i];
    EXPECT_EQ(2000+i, s.at) << i;
    EXPECT_EQ(i/50.0, s.value) << i;
  }
}

TEST_F(LastTest, TestLastChopsHead)
{
  auto& last = add("time-series/last")
                   .set("samples", 25.0);

  auto datacs = vector<DataCollection>(1);
  auto& datac = datacs[0];
  DataSet data;
  data.name = "Test";
  data.source = "TEST";
  for(auto i=0u; i<50; i++)
    data.add(2000+i, i/50.0);
  datac.add(data);

  auto& ds = add_source(datacs);
  ds.connect("output", last, "input");

  auto rdatacs = vector<DataCollection>{};
  auto& sink = add_sink(rdatacs, sample_rate);
  last.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, rdatacs.size());
  const auto& rdatac = rdatacs[0];
  ASSERT_EQ(1, rdatac.datasets.size());
  const auto& rdata = rdatac.datasets[0];
  ASSERT_EQ(25, rdata.samples.size());
  for(auto i=0u; i<rdata.samples.size(); i++)
  {
    const auto& s = rdata.samples[i];
    EXPECT_EQ(2025+i, s.at) << i;
    EXPECT_NEAR((i+25)/50.0, s.value, 1e-8) << i;
  }
  EXPECT_EQ("Test", rdata.name);
  EXPECT_EQ("TEST, last(25)", rdata.source);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
