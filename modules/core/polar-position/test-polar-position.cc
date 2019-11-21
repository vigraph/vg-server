//==========================================================================
// ViGraph dataflow module: core/polar-position/test-polar-position.cc
//
// Tests for polar-position filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"
#include <cmath>

class PolarPositionTest: public GraphTester
{
public:
  PolarPositionTest()
  {
    loader.load("./vg-module-core-polar-position.so");
  }
};

const auto nsamples = 1;

TEST_F(PolarPositionTest, TestDefaultis0_0)
{
  const auto expected_x = vector<double>(nsamples, 0);
  const auto expected_y = vector<double>(nsamples, 0);

  auto& pp = add("core/polar-position");
  auto actual_x = vector<double>{};
  auto& sink_x = add_sink(actual_x, nsamples);
  pp.connect("x", sink_x, "input");

  auto actual_y = vector<double>{};
  auto& sink_y = add_sink(actual_y, nsamples);
  pp.connect("y", sink_y, "input");

  run();

  EXPECT_EQ(expected_x, actual_x);
  EXPECT_EQ(expected_y, actual_y);
}

TEST_F(PolarPositionTest, TestAngle0IsX)
{
  const auto expected_x = vector<double>(nsamples, 1.0);
  const auto expected_y = vector<double>(nsamples, 0);

  auto& pp = add("core/polar-position").set("distance", 1.0);
  auto actual_x = vector<double>{};
  auto& sink_x = add_sink(actual_x, nsamples);
  pp.connect("x", sink_x, "input");

  auto actual_y = vector<double>{};
  auto& sink_y = add_sink(actual_y, nsamples);
  pp.connect("y", sink_y, "input");

  run();

  EXPECT_EQ(expected_x, actual_x);
  EXPECT_EQ(expected_y, actual_y);
}

TEST_F(PolarPositionTest, TestAngle0_25IsY)
{
  const auto expected_x = vector<double>(nsamples, 0);
  const auto expected_y = vector<double>(nsamples, 1);

  auto& pp = add("core/polar-position").set("angle", 0.25).set("distance", 1.0);
  auto actual_x = vector<double>{};
  auto& sink_x = add_sink(actual_x, nsamples);
  pp.connect("x", sink_x, "input");

  auto actual_y = vector<double>{};
  auto& sink_y = add_sink(actual_y, nsamples);
  pp.connect("y", sink_y, "input");

  run();

  EXPECT_NEAR(expected_x[0], actual_x[0], 1e-8);
  EXPECT_NEAR(expected_y[0], actual_y[0], 1e-8);
}

TEST_F(PolarPositionTest, TestAngle0_5Distance2IsMinus2X)
{
  const auto expected_x = vector<double>(nsamples, -2);
  const auto expected_y = vector<double>(nsamples, 0);

  auto& pp = add("core/polar-position").set("angle", 0.5).set("distance", 2.0);
  auto actual_x = vector<double>{};
  auto& sink_x = add_sink(actual_x, nsamples);
  pp.connect("x", sink_x, "input");

  auto actual_y = vector<double>{};
  auto& sink_y = add_sink(actual_y, nsamples);
  pp.connect("y", sink_y, "input");

  run();

  EXPECT_NEAR(expected_x[0], actual_x[0], 1e-8);
  EXPECT_NEAR(expected_y[0], actual_y[0], 1e-8);
}

TEST_F(PolarPositionTest, TestAngle0_75IsMinusY)
{
  const auto expected_x = vector<double>(nsamples, 0);
  const auto expected_y = vector<double>(nsamples, -1);

  auto& pp = add("core/polar-position").set("angle", 0.75).set("distance", 1.0);
  auto actual_x = vector<double>{};
  auto& sink_x = add_sink(actual_x, nsamples);
  pp.connect("x", sink_x, "input");

  auto actual_y = vector<double>{};
  auto& sink_y = add_sink(actual_y, nsamples);
  pp.connect("y", sink_y, "input");

  run();

  EXPECT_NEAR(expected_x[0], actual_x[0], 1e-8);
  EXPECT_NEAR(expected_y[0], actual_y[0], 1e-8);
}

TEST_F(PolarPositionTest, TestAngle1IsX)
{
  const auto expected_x = vector<double>(nsamples, 1.0);
  const auto expected_y = vector<double>(nsamples, 0);

  auto& pp = add("core/polar-position").set("angle", 1.0).set("distance", 1.0);
  auto actual_x = vector<double>{};
  auto& sink_x = add_sink(actual_x, nsamples);
  pp.connect("x", sink_x, "input");

  auto actual_y = vector<double>{};
  auto& sink_y = add_sink(actual_y, nsamples);
  pp.connect("y", sink_y, "input");

  run();

  EXPECT_NEAR(expected_x[0], actual_x[0], 1e-8);
  EXPECT_NEAR(expected_y[0], actual_y[0], 1e-8);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
