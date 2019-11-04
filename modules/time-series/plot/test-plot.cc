//==========================================================================
// ViGraph dataflow module: time-series/plot/test-plot.cc
//
// Tests for plot time-series filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../time-series-module.h"
#include "../../vector/vector-module.h"
#include "../../module-test.h"

class PlotTest: public GraphTester
{
public:
  PlotTest()
  {
    loader.load("./vg-module-time-series-plot.so");
  }
};

const auto sample_rate = 1;

TEST_F(PlotTest, TestSimpleSeriesAutoscaled)
{
  auto& plot = add("time-series/plot");

  auto datacs = vector<DataCollection>(1);
  auto& datac = datacs[0];
  DataSet data;
  for(auto i=0u; i<50; i++)
    data.add(2000+(50*i/49.0), 42.0*i/49.0);
  datac.add(data);

  auto& ds = add_source(datacs);
  ds.connect("output", plot, "input");

  auto frames = vector<Frame>{};
  auto& sink = add_sink(frames, sample_rate);
  plot.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(101, frame.points.size());
  for(auto i=0u; i<frame.points.size(); i++)
  {
    auto ai = i;
    if (i) ai--;  // Blank point

    // Two points the same because 50 spread over 100
    const auto& p = frame.points[i];
    EXPECT_NEAR(-0.5+(ai&~1)/98.0, p.x, 1e-6) << ai;
    EXPECT_NEAR(-0.5+(ai&~1)/98.0, p.y, 1e-6) << ai;
    EXPECT_EQ(i?Colour::white:Colour::black, p.c) << ai;
  }
}

TEST_F(PlotTest, TestSimpleSeriesNotAutoScaled)
{
  auto& plot = add("time-series/plot")
              .set("auto", false);

  auto datacs = vector<DataCollection>(1);
  auto& datac = datacs[0];
  DataSet data;
  for(auto i=0u; i<50; i++)
    data.add(2000+(50*i/49.0), 0.42*i/49.0);
  datac.add(data);

  auto& ds = add_source(datacs);
  ds.connect("output", plot, "input");

  auto frames = vector<Frame>{};
  auto& sink = add_sink(frames, sample_rate);
  plot.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, frames.size());
  const auto& frame = frames[0];
  ASSERT_EQ(101, frame.points.size());
  for(auto i=0u; i<frame.points.size(); i++)
  {
    auto ai = i;
    if (i) ai--;  // Blank point

    // Two points the same because 50 spread over 100
    const auto& p = frame.points[i];
    EXPECT_NEAR(-0.5+(ai&~1)     /98.0, p.x, 1e-6) << ai;
    EXPECT_NEAR(-0.5+(ai&~1)*0.42/98.0, p.y, 1e-6) << ai;
    EXPECT_EQ(i?Colour::white:Colour::black, p.c) << ai;
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
