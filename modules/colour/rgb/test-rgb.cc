//==========================================================================
// ViGraph dataflow module: colour/rgb/test-rgb.cc
//
// Tests for <rgb> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../colour-module.h"
#include "../../module-test.h"

class RGBTest: public GraphTester
{
public:
  RGBTest()
  {
    loader.load("./vg-module-colour-rgb.so");
  }
};

const auto sample_rate = 1;

TEST_F(RGBTest, TestDefaultRGBIsBlack)
{
  auto& trans = add("colour/rgb");

  auto outcs = vector<Colour::RGB>{};
  auto& sink = add_sink(outcs, sample_rate);
  trans.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, outcs.size());
  for(const auto& outc: outcs)
    EXPECT_EQ(Colour::black, outc);
}

TEST_F(RGBTest, TestSetRGB)
{
  auto& trans = add("colour/rgb").set("r", 0.1).set("g",0.2).set("b",0.3);

  auto outcs = vector<Colour::RGB>{};
  auto& sink = add_sink(outcs, sample_rate);
  trans.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, outcs.size());
  for(const auto& outc: outcs)
    EXPECT_EQ(Colour::RGB(0.1, 0.2, 0.3), outc);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
