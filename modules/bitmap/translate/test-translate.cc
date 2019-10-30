//==========================================================================
// ViGraph dataflow module: bitmap/translate/test-translate.cc
//
// Tests for <translate> filter
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"
#include "../../module-test.h"

class TranslateTest: public GraphTester
{
public:
  TranslateTest()
  {
    loader.load("./vg-module-bitmap-translate.so");
  }
};

const auto sample_rate = 1;

TEST_F(TranslateTest, TestDefaultTranslateHasNoEffect)
{
  auto& trans = add("bitmap/translate");

  auto bg_data = vector<Bitmap::Group>(1);
  auto& bg = bg_data[0];

  bg.add(Bitmap::Rectangle());

  auto& bgs = add_source(bg_data);
  bgs.connect("output", trans, "input");

  auto outbgs = vector<Bitmap::Group>{};
  auto& snk = add_sink(outbgs, sample_rate);
  trans.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outbgs.size());
  const auto& outbg = outbgs[0];
  ASSERT_EQ(1, outbg.items.size());
  auto pos = outbg.items[0].pos;
  EXPECT_EQ(0, pos.x);
  EXPECT_EQ(0, pos.y);
  EXPECT_EQ(0, pos.z);
}

TEST_F(TranslateTest, TestTranslateXYZ)
{
  auto& trans = add("bitmap/translate")
                .set("x", 1.0)
                .set("y", 2.0)
                .set("z", 3.0);

  auto bg_data = vector<Bitmap::Group>(1);
  auto& bg = bg_data[0];

  bg.add(Bitmap::Rectangle());

  auto& bgs = add_source(bg_data);
  bgs.connect("output", trans, "input");

  auto outbgs = vector<Bitmap::Group>{};
  auto& snk = add_sink(outbgs, sample_rate);
  trans.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, outbgs.size());
  const auto& outbg = outbgs[0];
  ASSERT_EQ(1, outbg.items.size());
  auto pos = outbg.items[0].pos;
  EXPECT_EQ(1, pos.x);
  EXPECT_EQ(2, pos.y);
  EXPECT_EQ(3, pos.z);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
