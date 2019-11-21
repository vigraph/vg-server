//==========================================================================
// ViGraph dataflow module: bitmap/twist/test-twist.cc
//
// Tests for <twist> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"
#include "../../module-test.h"

class TwistTest: public GraphTester
{
public:
  TwistTest()
  {
    loader.load("./vg-module-bitmap-twist.so");
  }
};

const auto sample_rate = 1;

TEST_F(TwistTest, TestDefaultTwistHasNoEffect)
{
  auto& trans = add("bitmap/twist");

  auto bg_data = vector<Bitmap::Group>(1);
  auto& bg = bg_data[0];

  Bitmap::Rectangle rect(5,3);
  rect.set(1,1, Colour::red);
  bg.add(rect);

  auto& bgs = add_source(bg_data);
  bgs.connect("output", trans, "input");

  auto outbgs = vector<Bitmap::Group>{};
  auto& snk = add_sink(outbgs, sample_rate);
  trans.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outbgs.size());
  const auto& outbg = outbgs[0];
  ASSERT_EQ(1, outbg.items.size());
  auto& outrect = outbg.items[0].rect;
  EXPECT_EQ(5, outrect.get_width());
  EXPECT_EQ(3, outrect.get_height());
  auto pos = outbg.items[0].pos;
  EXPECT_EQ(0, pos.x);
  EXPECT_EQ(0, pos.y);
  EXPECT_EQ(0, pos.z);

  for(auto i=0u; i<5; i++)
    for(auto j=0u; j<3; j++)
      EXPECT_EQ((i==1 && j==1)?Colour::red:Colour::black,
                outrect(i,j)) << i << "," << j;

}

TEST_F(TwistTest, TestTwistXAndY)
{
  // Note fractional y
  auto& trans = add("bitmap/twist").set("x", 2.0).set("y", 1.5);

  auto bg_data = vector<Bitmap::Group>(1);
  auto& bg = bg_data[0];

  Bitmap::Rectangle rect(5,3);
  rect.set(2,1, Colour::red);
  bg.add(rect);

  auto& bgs = add_source(bg_data);
  bgs.connect("output", trans, "input");

  auto outbgs = vector<Bitmap::Group>{};
  auto& snk = add_sink(outbgs, sample_rate);
  trans.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outbgs.size());
  const auto& outbg = outbgs[0];
  ASSERT_EQ(1, outbg.items.size());
  auto& outrect = outbg.items[0].rect;
  EXPECT_EQ(5, outrect.get_width());
  EXPECT_EQ(3, outrect.get_height());
  auto pos = outbg.items[0].pos;
  EXPECT_EQ(0, pos.x);
  EXPECT_EQ(0, pos.y);
  EXPECT_EQ(0, pos.z);

  for(auto i=0u; i<5; i++)
    for(auto j=0u; j<3; j++)
      EXPECT_EQ((i==0 && j==0)?Colour::red:Colour::black,
                outrect(i,j)) << i << "," << j;

}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
