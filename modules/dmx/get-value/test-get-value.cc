//==========================================================================
// ViGraph dataflow module: dmx/get-value/test-get-value.cc
//
// Tests for dmx/get-value
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../dmx-module.h"
#include "../../module-test.h"

class GetValueTest: public GraphTester
{
public:
  GetValueTest()
  {
    loader.load("./vg-module-dmx-get-value.so");
  }
};

const auto sample_rate = 1;

TEST_F(GetValueTest, TestWithNoInput)
{
  auto& getv = add("dmx/get-value")
    .set("universe", Integer{10})
    .set("channel", Integer{7});

  auto output = vector<Number>{};
  auto& sink = add_sink(output, sample_rate);
  getv.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, output.size());
  EXPECT_EQ(0, output[0]);
}

TEST_F(GetValueTest, TestWithInput)
{
  auto& getv = add("dmx/get-value")
    .set("universe", Integer{10})
    .set("channel", Integer{7});

  auto input_data = vector<DMX::State>(1);
  auto& state = input_data[0];
  state.set(DMX::channel_number(10, 7), 127);
  auto& is = add_source(input_data);
  is.connect("output", getv, "input");

  auto output = vector<Number>{};
  auto& sink = add_sink(output, sample_rate);
  getv.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, output.size());
  EXPECT_DOUBLE_EQ(127.0/255.0, output[0]);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
