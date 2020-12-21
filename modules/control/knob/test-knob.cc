//==========================================================================
// ViGraph dataflow module: control/knob/test-knob.cc
//
// Tests for <knob> control
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class KnobTest: public GraphTester
{
public:
  KnobTest()
  {
    loader.load("./vg-module-control-knob.so");
  }
};

const auto sample_rate = 1;

TEST_F(KnobTest, TestValueIsPassedThrough)
{
  auto& knob = add("control/knob").set("value", 42.0);
  auto outvs = vector<Number>{};
  auto& sink = add_sink(outvs, sample_rate);
  knob.connect("output", sink, "input");

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
