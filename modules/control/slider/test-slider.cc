//==========================================================================
// ViGraph dataflow module: control/slider/test-slider.cc
//
// Tests for <slider> control
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class SliderTest: public GraphTester
{
public:
  SliderTest()
  {
    loader.load("./vg-module-control-slider.so");
  }
};

const auto sample_rate = 1;

TEST_F(SliderTest, TestValueIsPassedThrough)
{
  auto& slider = add("control/slider").set("value", 42.0);
  auto outvs = vector<Number>{};
  auto& sink = add_sink(outvs, sample_rate);
  slider.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, outvs.size());
  for(const auto& outv: outvs)
    EXPECT_EQ(42, outv);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
