//==========================================================================
// ViGraph dataflow module: vector/blend/test-blend.cc
//
// Tests for blend bitmap source
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"
#include "../../module-test.h"

class BlendTest: public GraphTester
{
public:
  BlendTest()
  {
    loader.load("./vg-module-bitmap-blend.so");
  }
};

const auto sample_rate = 1;

TEST_F(BlendTest, TestDefaultBlendIs1x1SameAsTopleft)
{
  auto& blend = add("bitmap/blend");

  auto bg_data = vector<Bitmap::Group>(1);
  auto& bg = bg_data[0];

  Bitmap::Rectangle rect(5,3);
  rect.fill(Colour::red);
  bg.add(rect);

  auto& bgs = add_source(bg_data);
  bgs.connect("output", blend, "top-left");

  auto bitmaps = vector<Bitmap::Group>{};
  auto& snk = add_sink(bitmaps, sample_rate);
  blend.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, bitmaps.size());
  const auto& bitmap = bitmaps[0];
  ASSERT_EQ(1, bitmap.items.size());

  const auto& b0 = bitmap.items[0];
  EXPECT_EQ(0, b0.pos.x);
  EXPECT_EQ(0, b0.pos.y);
  EXPECT_EQ(0, b0.pos.z);
  EXPECT_EQ(1, b0.rect.get_width());
  EXPECT_EQ(1, b0.rect.get_height());
  EXPECT_EQ(Colour::red, b0.rect(0,0));
}

TEST_F(BlendTest, TestSpecifiedHeightAndWidth)
{
  auto& blend = add("bitmap/blend")
              .set("width", 5.0)
              .set("height", 3.0);

  auto bg_data = vector<Bitmap::Group>(1);
  auto& bg = bg_data[0];

  Bitmap::Rectangle rect(1,1);
  rect.fill(Colour::red);
  bg.add(rect);

  auto& bgs = add_source(bg_data);
  bgs.connect("output", blend, "top-left");

  auto bitmaps = vector<Bitmap::Group>{};
  auto& snk = add_sink(bitmaps, sample_rate);
  blend.connect("output", snk, "input");

  run();

  ASSERT_EQ(sample_rate, bitmaps.size());
  const auto& bitmap = bitmaps[0];
  ASSERT_EQ(1, bitmap.items.size());

  const auto& b0 = bitmap.items[0];
  EXPECT_EQ(0, b0.pos.x);
  EXPECT_EQ(0, b0.pos.y);
  EXPECT_EQ(0, b0.pos.z);
  ASSERT_EQ(5, b0.rect.get_width());
  ASSERT_EQ(3, b0.rect.get_height());

  for(auto i=0u; i<5; i++)
    for(auto j=0u; j<3; j++)
      EXPECT_EQ(Colour::red, b0.rect(i,j));
}

TEST_F(BlendTest, TestHorizontalBlend)
{
  auto& blend = add("bitmap/blend")
                .set("width", 5.0);

  // top-left
  auto bgtl_data = vector<Bitmap::Group>(1);
  auto& bgtl = bgtl_data[0];

  Bitmap::Rectangle recttl(1,1);
  recttl.fill(Colour::black);
  bgtl.add(recttl);

  auto& bgtls = add_source(bgtl_data);
  bgtls.connect("output", blend, "top-left");

  // top-right
  auto bgtr_data = vector<Bitmap::Group>(1);
  auto& bgtr = bgtr_data[0];

  Bitmap::Rectangle recttr(1,1);
  recttr.fill(Colour::red);
  bgtr.add(recttr);

  auto& bgtrs = add_source(bgtr_data);
  bgtrs.connect("output", blend, "top-right");

  // sink
  auto bitmaps = vector<Bitmap::Group>{};
  auto& sink = add_sink(bitmaps, sample_rate);
  blend.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, bitmaps.size());
  const auto& bitmap = bitmaps[0];
  ASSERT_EQ(1, bitmap.items.size());

  const auto& b0 = bitmap.items[0];
  EXPECT_EQ(0, b0.pos.x);
  EXPECT_EQ(0, b0.pos.y);
  EXPECT_EQ(0, b0.pos.z);
  ASSERT_EQ(5, b0.rect.get_width());
  ASSERT_EQ(1, b0.rect.get_height());

  for(auto i=0u; i<5; i++)
  {
    auto c = b0.rect(i, 0);
    EXPECT_NEAR(i/4.0, c.r, 1e-8) << i;
    EXPECT_EQ(0, c.g) << i;
    EXPECT_EQ(0, c.b) << i;
  }
}

TEST_F(BlendTest, TestVerticalBlend)
{
  auto& blend = add("bitmap/blend")
                .set("height", 5.0);

  // top-left
  auto bgtl_data = vector<Bitmap::Group>(1);
  auto& bgtl = bgtl_data[0];

  Bitmap::Rectangle recttl(1,1);
  recttl.fill(Colour::black);
  bgtl.add(recttl);

  auto& bgtls = add_source(bgtl_data);
  bgtls.connect("output", blend, "top-left");

  // bottom-left
  auto bgbl_data = vector<Bitmap::Group>(1);
  auto& bgbl = bgbl_data[0];

  Bitmap::Rectangle rectbl(1,1);
  rectbl.fill(Colour::red);
  bgbl.add(rectbl);

  auto& bgbls = add_source(bgbl_data);
  bgbls.connect("output", blend, "bottom-left");

  // sink
  auto bitmaps = vector<Bitmap::Group>{};
  auto& sink = add_sink(bitmaps, sample_rate);
  blend.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, bitmaps.size());
  const auto& bitmap = bitmaps[0];
  ASSERT_EQ(1, bitmap.items.size());

  const auto& b0 = bitmap.items[0];
  EXPECT_EQ(0, b0.pos.x);
  EXPECT_EQ(0, b0.pos.y);
  EXPECT_EQ(0, b0.pos.z);
  ASSERT_EQ(1, b0.rect.get_width());
  ASSERT_EQ(5, b0.rect.get_height());

  for(auto i=0u; i<5; i++)
  {
    auto c = b0.rect(0, i);
    EXPECT_NEAR(i/4.0, c.r, 1e-8) << i;
    EXPECT_EQ(0, c.g) << i;
    EXPECT_EQ(0, c.b) << i;
  }
}

TEST_F(BlendTest, Test2DBlend3Corners)
{
  auto& blend = add("bitmap/blend")
                .set("width", 5.0)
                .set("height", 3.0);

  // top-left
  auto bgtl_data = vector<Bitmap::Group>(1);
  auto& bgtl = bgtl_data[0];

  Bitmap::Rectangle recttl(1,1);
  recttl.fill(Colour::black);
  bgtl.add(recttl);

  auto& bgtls = add_source(bgtl_data);
  bgtls.connect("output", blend, "top-left");

  // top-right
  auto bgtr_data = vector<Bitmap::Group>(1);
  auto& bgtr = bgtr_data[0];

  Bitmap::Rectangle recttr(1,1);
  recttr.fill(Colour::red);
  bgtr.add(recttr);

  auto& bgtrs = add_source(bgtr_data);
  bgtrs.connect("output", blend, "top-right");

  // bottom-left
  auto bgbl_data = vector<Bitmap::Group>(1);
  auto& bgbl = bgbl_data[0];

  Bitmap::Rectangle rectbl(1,1);
  rectbl.fill(Colour::blue);
  bgbl.add(rectbl);

  auto& bgbls = add_source(bgbl_data);
  bgbls.connect("output", blend, "bottom-left");

  // sink
  auto bitmaps = vector<Bitmap::Group>{};
  auto& sink = add_sink(bitmaps, sample_rate);
  blend.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, bitmaps.size());
  const auto& bitmap = bitmaps[0];
  ASSERT_EQ(1, bitmap.items.size());

  const auto& b0 = bitmap.items[0];
  EXPECT_EQ(0, b0.pos.x);
  EXPECT_EQ(0, b0.pos.y);
  EXPECT_EQ(0, b0.pos.z);
  ASSERT_EQ(5, b0.rect.get_width());
  ASSERT_EQ(3, b0.rect.get_height());

  for(auto i=0u; i<5; i++)
  {
    for(auto j=0u; j<3; j++)
    {
      auto c = b0.rect(i, j);
      EXPECT_NEAR(i/4.0*(2.0-j)/2.0, c.r, 1e-8) << i << "," << j;
      EXPECT_EQ(0, c.g) << i << "," << j;
      EXPECT_NEAR(j/2.0, c.b, 1e-8) << i << "," << j;
    }
  }
}

TEST_F(BlendTest, Test2DBlend4Corners)
{
  auto& blend = add("bitmap/blend")
                .set("width", 5.0)
                .set("height", 3.0);

  // top-left
  auto bgtl_data = vector<Bitmap::Group>(1);
  auto& bgtl = bgtl_data[0];

  Bitmap::Rectangle recttl(1,1);
  recttl.fill(Colour::black);
  bgtl.add(recttl);

  auto& bgtls = add_source(bgtl_data);
  bgtls.connect("output", blend, "top-left");

  // top-right
  auto bgtr_data = vector<Bitmap::Group>(1);
  auto& bgtr = bgtr_data[0];

  Bitmap::Rectangle recttr(1,1);
  recttr.fill(Colour::red);
  bgtr.add(recttr);

  auto& bgtrs = add_source(bgtr_data);
  bgtrs.connect("output", blend, "top-right");

  // bottom-left
  auto bgbl_data = vector<Bitmap::Group>(1);
  auto& bgbl = bgbl_data[0];

  Bitmap::Rectangle rectbl(1,1);
  rectbl.fill(Colour::blue);
  bgbl.add(rectbl);

  auto& bgbls = add_source(bgbl_data);
  bgbls.connect("output", blend, "bottom-left");

  // bottom-right
  auto bgbr_data = vector<Bitmap::Group>(1);
  auto& bgbr = bgbr_data[0];

  Bitmap::Rectangle rectbr(1,1);
  rectbr.fill(Colour::green);
  bgbr.add(rectbr);

  auto& bgbrs = add_source(bgbr_data);
  bgbrs.connect("output", blend, "bottom-right");

  // sink
  auto bitmaps = vector<Bitmap::Group>{};
  auto& sink = add_sink(bitmaps, sample_rate);
  blend.connect("output", sink, "input");

  run();

  ASSERT_EQ(sample_rate, bitmaps.size());
  const auto& bitmap = bitmaps[0];
  ASSERT_EQ(1, bitmap.items.size());

  const auto& b0 = bitmap.items[0];
  EXPECT_EQ(0, b0.pos.x);
  EXPECT_EQ(0, b0.pos.y);
  EXPECT_EQ(0, b0.pos.z);
  ASSERT_EQ(5, b0.rect.get_width());
  ASSERT_EQ(3, b0.rect.get_height());

  for(auto i=0u; i<5; i++)
  {
    for(auto j=0u; j<3; j++)
    {
      auto c = b0.rect(i, j);
      EXPECT_NEAR(i/4.0*(2.0-j)/2.0, c.r, 1e-8) << i << "," << j;
      EXPECT_NEAR(j/2.0*i/4.0, c.g, 1e-8) << i << "," << j;
      EXPECT_NEAR(j/2.0*(4.0-i)/4.0, c.b, 1e-8) << i << "," << j;
    }
  }
}

int main(int argc, char **argv)
{
  if (argc > 1 && string(argv[1]) == "-v")
  {
    auto chan_out = new Log::StreamChannel{&cout};
    Log::logger.connect(chan_out);
  }

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
