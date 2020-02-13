//==========================================================================
// ViGraph dataflow module: maths/differentiate/test-differentiate.cc
//
// Tests for differentiate control
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class DifferentiateTest: public GraphTester
{
public:
  DifferentiateTest()
  {
    loader.load("./vg-module-maths-differentiate.so");
  }
};

const auto sample_rate = 1;

TEST_F(DifferentiateTest, TestDiffAndNoSpikeAtStart)
{
  auto& diff = add("maths/differentiate");

  auto input_data = vector<Number>(3);
  input_data[0] = 10.0;
  input_data[1] = 11.0;
  input_data[2] = 9.0;
  auto& is = add_source(input_data);
  is.connect("output", diff, "input");

  auto output = vector<Number>{};
  auto& sink = add_sink(output, sample_rate);
  diff.connect("output", sink, "input");

  run(3);

  EXPECT_EQ(3, output.size());
  EXPECT_EQ(0.0, output[0]);   // Spike prevention - not 10
  EXPECT_EQ(1.0, output[1]);   // 10 -> 11
  EXPECT_EQ(-2.0, output[2]);  // 11 -> 9
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
