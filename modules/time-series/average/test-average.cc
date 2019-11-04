//==========================================================================
// ViGraph dataflow module: time-series/average/test-average.cc
//
// Tests for average time-series filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../time-series-module.h"
#include "../../module-test.h"
#include <cmath>

class AverageTest: public GraphTester
{
public:
  AverageTest()
  {
    loader.load("./vg-module-time-series-average.so");
  }
};

const auto sample_rate = 1;

TEST_F(AverageTest, TestNoChangeIfSamples0)
{
  auto& average = add("time-series/average");

  auto datacs = vector<DataCollection>(1);
  auto& datac = datacs[0];
  DataSet data;
  for(auto i=0u; i<50; i++)
    data.add(2000+i, i/50.0);
  datac.add(data);

  auto& ds = add_source(datacs);
  ds.connect("output", average, "input");

  auto rdatacs = vector<DataCollection>{};
  auto& sink = add_sink(rdatacs, sample_rate);
  average.connect("output", sink, "input");

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

TEST_F(AverageTest, TestAverageEvery2)
{
  auto& average = add("time-series/average")
                   .set("samples", 2.0);

  auto datacs = vector<DataCollection>(1);
  auto& datac = datacs[0];
  DataSet data;
  data.name = "Test";
  data.source = "TEST";
  for(auto i=0u; i<50; i++)
    data.add(2000+i, i/50.0);
  datac.add(data);

  auto& ds = add_source(datacs);
  ds.connect("output", average, "input");

  auto rdatacs = vector<DataCollection>{};
  auto& sink = add_sink(rdatacs, sample_rate);
  average.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, rdatacs.size());
  const auto& rdatac = rdatacs[0];
  ASSERT_EQ(1, rdatac.datasets.size());
  const auto& rdata = rdatac.datasets[0];
  ASSERT_EQ(48, rdata.samples.size());  // Lost one off each end
  for(auto i=0u; i<rdata.samples.size(); i++)
  {
    const auto& s = rdata.samples[i];
    EXPECT_EQ(2001+i, s.at) << i;
    EXPECT_NEAR((i+i+1)/100.0, s.value, 1e-8) << i;
  }
  EXPECT_EQ("Test", rdata.name);
  EXPECT_EQ("TEST, average(2)", rdata.source);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
