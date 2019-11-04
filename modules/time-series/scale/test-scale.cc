//==========================================================================
// ViGraph dataflow module: time-series/scale/test-scale.cc
//
// Tests for scale time-series filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../time-series-module.h"
#include "../../module-test.h"
#include <cmath>

class ScaleTest: public GraphTester
{
public:
  ScaleTest()
  {
    loader.load("./vg-module-time-series-scale.so");
  }
};

const auto sample_rate = 1;

TEST_F(ScaleTest, TestNoChange)
{
  auto& scale = add("time-series/scale");

  auto datas = vector<DataSet>(1);
  auto& data = datas[0];
  for(auto i=0u; i<50; i++)
    data.add(2000+i, i/50.0);

  auto& ds = add_source(datas);
  ds.connect("output", scale, "input");

  auto rdatas = vector<DataSet>{};
  auto& sink = add_sink(rdatas, sample_rate);
  scale.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, rdatas.size());
  const auto& rdata = rdatas[0];
  ASSERT_EQ(50, rdata.samples.size());
  for(auto i=0u; i<rdata.samples.size(); i++)
  {
    const auto& s = rdata.samples[i];
    EXPECT_EQ(2000+i, s.at) << i;
    EXPECT_DOUBLE_EQ(i/50.0, s.value) << i;
  }
}

TEST_F(ScaleTest, TestWithChange)
{
  auto& scale = add("time-series/scale")
                   .set("factor", 10.0);

  auto datas = vector<DataSet>(1);
  auto& data = datas[0];
  for(auto i=0u; i<50; i++)
    data.add(2000+i, i/50.0);

  auto& ds = add_source(datas);
  ds.connect("output", scale, "input");

  auto rdatas = vector<DataSet>{};
  auto& sink = add_sink(rdatas, sample_rate);
  scale.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, rdatas.size());
  const auto& rdata = rdatas[0];
  ASSERT_EQ(50, rdata.samples.size());
  for(auto i=0u; i<rdata.samples.size(); i++)
  {
    const auto& s = rdata.samples[i];
    EXPECT_EQ(2000+i, s.at) << i;
    EXPECT_DOUBLE_EQ(10.0*i/50.0, s.value) << i;
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
