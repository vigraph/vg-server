//==========================================================================
// ViGraph dataflow module: colour/hsl/test-hsl.cc
//
// Tests for <hsl> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../colour-module.h"
#include "../../module-test.h"

class HSLTest: public GraphTester
{
public:
  HSLTest()
  {
    loader.load("./vg-module-colour-hsl.so");
  }
};

const auto sample_rate = 1;

TEST_F(HSLTest, TestDefaultHSLIsRed)
{
  auto& trans = add("colour/hsl");

  auto outcs = vector<Colour::RGB>{};
  auto& sink = add_sink(outcs, sample_rate);
  trans.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, outcs.size());
  for(const auto& outc: outcs)
    EXPECT_EQ(Colour::red, outc);
}

TEST_F(HSLTest, TestSetHSL)
{
  auto& trans = add("colour/hsl").set("h", 0.1).set("s",0.2).set("l",0.3);

  auto outcs = vector<Colour::RGB>{};
  auto& sink = add_sink(outcs, sample_rate);
  trans.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, outcs.size());
  for(const auto& outc: outcs)
    EXPECT_EQ(Colour::RGB(Colour::HSL(0.1, 0.2, 0.3)), outc);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
