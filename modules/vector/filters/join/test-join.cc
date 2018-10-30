//==========================================================================
// ViGraph dataflow module: vector/filters/join/test-join.cc
//
// Tests for <join> filter
//
// Copyright (c) 2017 Paul Clark.  All rights reserved
//==========================================================================

#include "../../vector-module-test.h"
ModuleLoader loader;

TEST(JoinTest, TestSingleInputPassesThrough)
{
  const string& xml = R"(
    <graph>
      <figure points='10'/>
      <join/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 10 points at (0,0)
  EXPECT_EQ(10, frame->points.size());
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(0.0, p.x);
    EXPECT_EQ(0.0, p.y);
    EXPECT_EQ(0.0, p.z);
  }
}

TEST(JoinTest, TestTwoInputsJoins)
{
  const string& xml = R"(
    <graph>
      <figure points='10'/>
      <figure points='10'>
        <x pos="1"/>
      </figure>
      <join/>
    </graph>
  )";

  FrameGenerator gen(xml, loader);
  Frame *frame = gen.get_frame();
  ASSERT_FALSE(!frame);

  // Should be 10 points at (0,0) then 10 at (1,0)
  EXPECT_EQ(20, frame->points.size());
  int i=0;
  for(const auto& p: frame->points)
  {
    EXPECT_EQ(i<10?0.0:1.0, p.x);
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
  loader.load("./vg-module-vector-filter-join.so");
  return RUN_ALL_TESTS();
}
