//==========================================================================
// ViGraph dataflow module: time-series/capture/test-capture.cc
//
// Tests for capture filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../time-series-module.h"
#include "../../module-test.h"

class CaptureTest: public GraphTester
{
public:
  CaptureTest()
  {
    loader.load("./vg-module-time-series-capture.so");
  }
};

const auto sample_rate = 1;

TEST_F(CaptureTest, TestCaptureFreeRunningAndStop)
{
  auto& cap = add("time-series/capture")
    .set("sample-rate", 1.0)
    .set("samples", Integer{2});

  auto input_data = vector<Number>(3);
  input_data[0] = 1.0;
  input_data[1] = 2.0;
  input_data[2] = 3.0;
  auto& is = add_source(input_data);
  is.connect("output", cap, "input");

  auto datacs = vector<DataCollection>{};
  auto& sink = add_sink(datacs, sample_rate);
  cap.connect("output", sink, "input");

  run(3);

  ASSERT_EQ(3, datacs.size());
  const auto& datac = datacs[2];
  ASSERT_EQ(1, datac.datasets.size());
  auto& data = datac.datasets[0];
  ASSERT_EQ(2, data.samples.size());

  EXPECT_EQ(1.0, data.samples[0].value);
  EXPECT_EQ(2.0, data.samples[1].value);
}

TEST_F(CaptureTest, TestCaptureRotate)
{
  auto& cap = add("time-series/capture")
    .set("sample-rate", 1.0)
    .set("samples", Integer{2})
    .set("rotate", true);

  auto input_data = vector<Number>(3);
  input_data[0] = 1.0;
  input_data[1] = 2.0;
  input_data[2] = 3.0;
  auto& is = add_source(input_data);
  is.connect("output", cap, "input");

  auto datacs = vector<DataCollection>{};
  auto& sink = add_sink(datacs, sample_rate);
  cap.connect("output", sink, "input");

  run(3);

  ASSERT_EQ(3, datacs.size());
  const auto& datac = datacs[2];
  ASSERT_EQ(1, datac.datasets.size());
  auto& data = datac.datasets[0];
  ASSERT_EQ(2, data.samples.size());

  EXPECT_EQ(2.0, data.samples[0].value);
  EXPECT_EQ(3.0, data.samples[1].value);
}

TEST_F(CaptureTest, TestCaptureManualStart)
{
  auto& cap = add("time-series/capture")
    .set("sample-rate", 1.0)
    .set("samples", Integer{2});

  auto input_data = vector<Number>(3);
  input_data[0] = 1.0;
  input_data[1] = 2.0;
  input_data[2] = 3.0;
  auto& is = add_source(input_data);
  is.connect("output", cap, "input");

  auto start_data = vector<Trigger>(3);
  start_data[1] = 1.0;
  auto& ss = add_source(start_data);
  ss.connect("output", cap, "start");

  auto datacs = vector<DataCollection>{};
  auto& sink = add_sink(datacs, sample_rate);
  cap.connect("output", sink, "input");

  run(3);

  ASSERT_EQ(3, datacs.size());
  const auto& datac = datacs[2];
  ASSERT_EQ(1, datac.datasets.size());
  auto& data = datac.datasets[0];
  ASSERT_EQ(2, data.samples.size());

  EXPECT_EQ(2.0, data.samples[0].value);
  EXPECT_EQ(3.0, data.samples[1].value);
}

TEST_F(CaptureTest, TestCaptureManualStop)
{
  auto& cap = add("time-series/capture")
    .set("sample-rate", 1.0)
    .set("samples", Integer{3});

  auto input_data = vector<Number>(3);
  input_data[0] = 1.0;
  input_data[1] = 2.0;
  input_data[2] = 3.0;
  auto& is = add_source(input_data);
  is.connect("output", cap, "input");

  auto stop_data = vector<Trigger>(3);
  stop_data[2] = 1.0;
  auto& ss = add_source(stop_data);
  ss.connect("output", cap, "stop");

  auto datacs = vector<DataCollection>{};
  auto& sink = add_sink(datacs, sample_rate);
  cap.connect("output", sink, "input");

  run(3);

  ASSERT_EQ(3, datacs.size());
  const auto& datac = datacs[2];
  ASSERT_EQ(1, datac.datasets.size());
  auto& data = datac.datasets[0];
  ASSERT_EQ(2, data.samples.size());

  EXPECT_EQ(1.0, data.samples[0].value);
  EXPECT_EQ(2.0, data.samples[1].value);
}

TEST_F(CaptureTest, TestCaptureManualClear)
{
  auto& cap = add("time-series/capture")
    .set("sample-rate", 1.0)
    .set("samples", Integer{3});

  auto input_data = vector<Number>(3);
  input_data[0] = 1.0;
  input_data[1] = 2.0;
  input_data[2] = 3.0;
  auto& is = add_source(input_data);
  is.connect("output", cap, "input");

  auto clear_data = vector<Trigger>(3);
  clear_data[2] = 1.0;
  auto& cs = add_source(clear_data);
  cs.connect("output", cap, "clear");

  auto datacs = vector<DataCollection>{};
  auto& sink = add_sink(datacs, sample_rate);
  cap.connect("output", sink, "input");

  run(3);

  ASSERT_EQ(3, datacs.size());
  const auto& datac = datacs[2];
  ASSERT_EQ(1, datac.datasets.size());
  auto& data = datac.datasets[0];
  ASSERT_EQ(1, data.samples.size());

  EXPECT_EQ(3.0, data.samples[0].value);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
