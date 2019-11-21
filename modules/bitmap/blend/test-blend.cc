//==========================================================================
// ViGraph dataflow module: vector/blend/test-blend.cc
//
// Tests for blend bitmap source
//
// Copyright (c) 2019 Paul Clark.  All rights reserved
//==========================================================================

#include "../bitmap-module.h"
#include "../../colour/colour-module.h"
#include "../../module-test.h"
#include "blend-type.h"

class BlendTest: public GraphTester
{
public:
  BlendTest()
  {
    loader.load("./vg-module-bitmap-blend.so");
  }
};

const auto sample_rate = 1;

TEST_F(BlendTest, TestHorizontal)
{
  auto& blend = add("bitmap/blend")
                .set("type", BlendType::horizontal)
                .set("width", 5.0)
                .set("height", 3.0);
  setup(blend);

  // left
  auto cl_data = vector<Colour::RGB>(1);
  cl_data[0] = Colour::black;
  auto& cls = add_source(cl_data);
  cls.connect("output", blend, "left");

  // right
  auto cr_data = vector<Colour::RGB>(1);
  cr_data[0] = Colour::red;
  auto& crs = add_source(cr_data);
  crs.connect("output", blend, "right");

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
      EXPECT_NEAR(i/4.0, c.r, 1e-8) << i << "," << j;
      EXPECT_EQ(0, c.g) << i << "," << j;
      EXPECT_EQ(0, c.b) << i << "," << j;
    }
  }
}

TEST_F(BlendTest, TestVertical)
{
  auto& blend = add("bitmap/blend")
                .set("type", BlendType::vertical)
                .set("width", 5.0)
                .set("height", 3.0);
  setup(blend);

  // top
  auto ct_data = vector<Colour::RGB>(1);
  ct_data[0] = Colour::black;
  auto& cts = add_source(ct_data);
  cts.connect("output", blend, "top");

  // bottom
  auto cb_data = vector<Colour::RGB>(1);
  cb_data[0] = Colour::red;
  auto& cbs = add_source(cb_data);
  cbs.connect("output", blend, "bottom");

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
      EXPECT_NEAR(j/2.0, c.r, 1e-8) << i << "," << j;
      EXPECT_EQ(0, c.g) << i << "," << j;
      EXPECT_EQ(0, c.b) << i << "," << j;
    }
  }
}

TEST_F(BlendTest, TestRectangular)
{
  auto& blend = add("bitmap/blend")
                .set("type", BlendType::rectangular)
                .set("width", 5.0)
                .set("height", 3.0);
  setup(blend);

  // top-left
  auto ctl_data = vector<Colour::RGB>(1);
  ctl_data[0] = Colour::black;
  auto& ctls = add_source(ctl_data);
  ctls.connect("output", blend, "top-left");

  // top-right
  auto ctr_data = vector<Colour::RGB>(1);
  ctr_data[0] = Colour::red;
  auto& ctrs = add_source(ctr_data);
  ctrs.connect("output", blend, "top-right");

  // bottom-left
  auto cbl_data = vector<Colour::RGB>(1);
  cbl_data[0] = Colour::blue;
  auto& cbls = add_source(cbl_data);
  cbls.connect("output", blend, "bottom-left");

  // bottom-right
  auto cbr_data = vector<Colour::RGB>(1);
  cbr_data[0] = Colour::green;
  auto& cbrs = add_source(cbr_data);
  cbrs.connect("output", blend, "bottom-right");

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
