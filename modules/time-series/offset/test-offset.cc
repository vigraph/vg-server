//==========================================================================
// ViGraph dataflow module: time-series/offset/test-offset.cc
//
// Tests for offset time-series filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../time-series-module.h"
#include "../../module-test.h"
#include <cmath>

class OffsetTest: public GraphTester
{
public:
  OffsetTest()
  {
    loader.load("./vg-module-time-series-offset.so");
  }
};

const auto sample_rate = 1;

TEST_F(OffsetTest, TestNoChange)
{
  auto& offset = add("time-series/offset");

  auto datas = vector<DataSet>(1);
  auto& data = datas[0];
  for(auto i=0u; i<50; i++)
    data.add(2000+i, i/50.0);

  auto& ds = add_source(datas);
  ds.connect("output", offset, "input");

  auto rdatas = vector<DataSet>{};
  auto& sink = add_sink(rdatas, sample_rate);
  offset.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, rdatas.size());
  const auto& rdata = rdatas[0];
  ASSERT_EQ(50, rdata.samples.size());
  for(auto i=0u; i<rdata.samples.size(); i++)
  {
    const auto& s = rdata.samples[i];
    EXPECT_EQ(2000+i, s.at) << i;
    EXPECT_EQ(i/50.0, s.value) << i;
  }
}

TEST_F(OffsetTest, TestWithChange)
{
  auto& offset = add("time-series/offset")
                   .set("amount", 10.0);

  auto datas = vector<DataSet>(1);
  auto& data = datas[0];
  for(auto i=0u; i<50; i++)
    data.add(2000+i, i/50.0);

  auto& ds = add_source(datas);
  ds.connect("output", offset, "input");

  auto rdatas = vector<DataSet>{};
  auto& sink = add_sink(rdatas, sample_rate);
  offset.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, rdatas.size());
  const auto& rdata = rdatas[0];
  ASSERT_EQ(50, rdata.samples.size());
  for(auto i=0u; i<rdata.samples.size(); i++)
  {
    const auto& s = rdata.samples[i];
    EXPECT_EQ(2000+i, s.at) << i;
    EXPECT_EQ(10.0+i/50.0, s.value) << i;
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
