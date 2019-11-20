//==========================================================================
// ViGraph dataflow module: colour/blend/test-blend.cc
//
// Tests for <blend> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../colour-module.h"
#include "../../module-test.h"

class BlendTest: public GraphTester
{
public:
  BlendTest()
  {
    loader.load("./vg-module-colour-blend.so");
  }
};

const auto sample_rate = 1;

TEST_F(BlendTest, TestDefaultBlendIsFrom)
{
  auto& blend = add("colour/blend");

  auto from_data = vector<Colour::RGB>(1);
  from_data[0] = Colour::red;
  auto& froms = add_source(from_data);
  froms.connect("output", blend, "from");

  auto to_data = vector<Colour::RGB>(1);
  to_data[0] = Colour::blue;
  auto& tos = add_source(to_data);
  tos.connect("output", blend, "to");

  auto outcs = vector<Colour::RGB>{};
  auto& sink = add_sink(outcs, sample_rate);
  blend.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, outcs.size());
  for(const auto& outc: outcs)
    EXPECT_EQ(Colour::red, outc);
}

TEST_F(BlendTest, TestHalfBlendIsMidway)
{
  auto& blend = add("colour/blend").set("mix", 0.5);

  auto from_data = vector<Colour::RGB>(1);
  from_data[0] = Colour::red;
  auto& froms = add_source(from_data);
  froms.connect("output", blend, "from");

  auto to_data = vector<Colour::RGB>(1);
  to_data[0] = Colour::blue;
  auto& tos = add_source(to_data);
  tos.connect("output", blend, "to");

  auto outcs = vector<Colour::RGB>{};
  auto& sink = add_sink(outcs, sample_rate);
  blend.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, outcs.size());
  for(const auto& outc: outcs)
    EXPECT_EQ(Colour::RGB(0.5, 0, 0.5), outc);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
