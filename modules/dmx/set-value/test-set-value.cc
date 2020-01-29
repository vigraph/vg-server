//==========================================================================
// ViGraph dataflow module: dmx/set-value/test-set-value.cc
//
// Tests for dmx/set-value
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../dmx-module.h"
#include "../../module-test.h"

class SetValueTest: public GraphTester
{
public:
  SetValueTest()
  {
    loader.load("./vg-module-dmx-set-value.so");
  }
};

const auto sample_rate = 1;

TEST_F(SetValueTest, TestWithNoInput)
{
  auto& setv = add("dmx/set-value")
    .set("universe", Integer{10})
    .set("channel", Integer{7});

  auto states = vector<DMX::State>{};
  auto& sink = add_sink(states, sample_rate);
  setv.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, states.size());
  auto& state = states[0];
  auto vchan = DMX::channel_number(10, 7);
  ASSERT_EQ(1, state.regions.size());
  ASSERT_EQ(1, state.regions[vchan].size()) << vchan;
  EXPECT_EQ(0, state.regions[vchan][0]);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
