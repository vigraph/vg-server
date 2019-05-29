//==========================================================================
// ViGraph dataflow module: vector/filters/combine/test-combine.cc
//
// Tests for <combine> filter
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module-test.h"
ModuleLoader loader;

TEST(CombineTest, TestSingleInputPassesThrough)
{
  FrameGraphTester tester{loader};

  auto figure = tester.add("figure").set("points", 10);
  auto combine = tester.add("combine");

  figure.connect("default", combine, "default");

  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 11 points at (0,0)
  EXPECT_EQ(11, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(0.0, p.x);
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
  }
}

TEST(CombineTest, TestTwoInputsCombines)
{
  FrameGraphTester tester{loader};

  auto figure1 = tester.add("figure").set("points", 10);
  auto figure2 = tester.add("figure").set("points", 10).set("x.pos", 1);
  auto combine = tester.add("combine");

  figure1.connect("default", combine, "default");
  figure2.connect("default", combine, "default");

  tester.run();
  Frame *frame = tester.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 11 points at (0,0) then 11 at (1,0)
  EXPECT_EQ(22, frame->points.size());
  int i=0;
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(i<11?0.0:1.0, p.x);
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
    i++;
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
  loader.load("../../sources/figure/vg-module-vector-source-figure.so");
  loader.load("./vg-module-vector-filter-combine.so");
  loader.add_default_section("vector");
  return RUN_ALL_TESTS();
}
