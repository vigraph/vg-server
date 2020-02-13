//==========================================================================
// ViGraph dataflow module: maths/integrate/test-integrate.cc
//
// Tests for integrate control
//
// Copyright (c) 2017-2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../../module-test.h"

class IntegrateTest: public GraphTester
{
public:
  IntegrateTest()
  {
    loader.load("./vg-module-maths-integrate.so");
  }
};

const auto sample_rate = 1;

TEST_F(IntegrateTest, TestIntegration)
{
  auto& integ = add("maths/integrate");

  auto input_data = vector<Number>(3);
  input_data[0] = 1.0;
  input_data[1] = 2.0;
  input_data[2] = -3.0;
  auto& is = add_source(input_data);
  is.connect("output", integ, "input");

  auto output = vector<Number>{};
  auto& sink = add_sink(output, sample_rate);
  integ.connect("output", sink, "input");

  run(3);

  EXPECT_EQ(3, output.size());
  EXPECT_EQ(1.0, output[0]);
  EXPECT_EQ(3.0, output[1]);
  EXPECT_EQ(0.0, output[2]);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
