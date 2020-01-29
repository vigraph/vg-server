//==========================================================================
// ViGraph dataflow module: colour/split/test-split.cc
//
// Tests for <split> filter
//
// Copyright (c) 2020 Paul Clark.  All rights reserved
//==========================================================================

#include "../colour-module.h"
#include "../../module-test.h"

class SplitTest: public GraphTester
{
public:
  SplitTest()
  {
    loader.load("./vg-module-colour-split.so");
  }
};

const auto sample_rate = 1;

TEST_F(SplitTest, TestDefaultSplitIsFrom)
{
  auto& split = add("colour/split");

  auto input_data = vector<Colour::RGB>(1);
  input_data[0] = Colour::RGB{0.1, 0.2, 0.3};
  auto& is = add_source(input_data);
  is.connect("output", split, "input");

  auto rs = vector<Number>{};
  auto& rsink = add_sink(rs, sample_rate);
  split.connect("red", rsink, "input");

  auto gs = vector<Number>{};
  auto& gsink = add_sink(gs, sample_rate);
  split.connect("green", gsink, "input");

  auto bs = vector<Number>{};
  auto& bsink = add_sink(bs, sample_rate);
  split.connect("blue", bsink, "input");

  run();

  ASSERT_EQ(sample_rate, rs.size());
  for(const auto& r: rs)
    EXPECT_EQ(0.1, r);
  ASSERT_EQ(sample_rate, gs.size());
  for(const auto& g: gs)
    EXPECT_EQ(0.2, g);
  ASSERT_EQ(sample_rate, bs.size());
  for(const auto& b: bs)
    EXPECT_EQ(0.3, b);
}


int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
